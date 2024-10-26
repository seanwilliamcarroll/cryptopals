#include <raw_bytes.hpp>

#include <doctest/doctest.h>
#include <rapidcheck.h>

namespace testing {

TEST_SUITE("crypt.raw_bytes") {

  TEST_CASE("sc::example") {
    // A simple doctest assertion:

    CHECK((1 * 3) == 3);

    // A simple RapidCheck property check:

    rc::check("∀i ∈ ℤ: example(i) == i * 3",
              [](int i) { return (3 * i) == i * 3; });
  }
}

} // namespace testing
