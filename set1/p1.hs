
-- Translate hex string to base64 string
-- Want to operate on actual bytes first

module Main where

import Data.Bits
import Data.Word

type HexChar = Word8
type HexString = [HexChar]

-- TODO: Need to enforce what values are legal?
type Base64Char = Word8
type Base64String = [Base64Char]

charToWord8 :: Char -> Word8
charToWord8 = toEnum . fromEnum

word8ToChar :: Word8 -> Char
word8ToChar = toEnum . fromEnum

inBound :: Word8 -> Word8 -> Word8 -> Bool
inBound lower upper val = (val >= lower) && (val <= upper)

word8ToHexChar :: Word8 -> HexChar
word8ToHexChar word8 =
  let
    capA = charToWord8 'A'
    capF = charToWord8 'F'
    lowA = charToWord8 'a'
    lowF = charToWord8 'f'
    zero = charToWord8 '0'
    nine = charToWord8 '9'
  in
    if inBound capA capF word8
    then (word8 - capA) + 10
    else if inBound lowA lowF word8
    then (word8 - lowA) + 10
    else if inBound zero nine word8
    then word8 - zero
    else error "Bad value passed for hex string"

-- TODO: More elegant solution?
word8ListToBase64String :: [Word8] -> Base64String
word8ListToBase64String (x : x' : x'' : xs) =
  let
    first = shiftR x 2
    second = (shiftL (x .&. 0x3) 4) .|. (shiftR (x' .&. 0xF0) 4)
    third = (shiftL (x' .&. 0xF) 2) .|. (shiftR x'' 6)
    fourth = x'' .&. 0x3F
  in
    first : second : third : fourth : word8ListToBase64String xs
word8ListToBase64String (x : x' : xs) =
  let
    first = shiftR x 2
    second = (shiftL (x .&. 0x3) 4) .|. (shiftR (x' .&. 0xF0) 4)
    third = (shiftL (x' .&. 0xF) 2)
  in
    first : second : third : word8ListToBase64String xs
word8ListToBase64String (x : xs) =
  let
    first = shiftR x 2
    second = (shiftL (x .&. 0x3) 4)
  in
    first : second : word8ListToBase64String xs
word8ListToBase64String [] = []

base64CharToChar :: Base64Char -> Char
base64CharToChar base64Char =
  let
    capA = 0
    capZ = 25
    lowA = 26
    lowZ = 51
    zero = 52
    nine = 61
    plus = 62
    slash = 63
    convertedWord8 = if inBound capA capZ base64Char
      then base64Char + (charToWord8 'A')
      else if inBound lowA lowZ base64Char
      then (base64Char - lowA) + (charToWord8 'a')
      else if inBound zero nine base64Char
      then (base64Char - zero) + (charToWord8 '0')
      else if base64Char == plus
      then charToWord8 '+'
      else if base64Char == slash
      then charToWord8 '/'
      else error "Bad value for Base64Char"
  in
    word8ToChar convertedWord8

base64StringToString :: Base64String -> [Char]
base64StringToString base64String =
  let
    leftover = mod (length base64String) 4
    convertedString = map base64CharToChar base64String
  in
    if leftover == 0
    then convertedString
    else if leftover == 3
    then convertedString ++ "="
    else if leftover == 2
    then convertedString ++ "=="
    else error "Base64String not converted correctly"

hexStringToWord8List :: HexString -> [Word8]
hexStringToWord8List hexString =
  let
    paddedHexString =
      if (mod (length hexString) 2) == 0
      then hexString
      else 0 : hexString
    converter :: HexString -> [Word8]
    converter (x : x' : xs) =
      let combinedValue = (shiftL x 4) .|. x'
      in combinedValue : converter xs
    converter [] = []
  in
    converter paddedHexString

demo :: [Char] -> [Char]
demo input =
  let
    convert :: Char -> Word8
    convert x = word8ToHexChar $ charToWord8 x
    hexString = map convert input
    rawBytes = hexStringToWord8List hexString
    base64String = word8ListToBase64String rawBytes
  in
    base64StringToString base64String

main :: IO ()

main = do
  line <- getLine
  putStrLn line
  print $ demo line

