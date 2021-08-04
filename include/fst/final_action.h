#pragma once
#include <fst/assert>
#include <type_traits>
#include <utility>

namespace fst {
/// final_action allows you to ensure something gets run at the end of a scope.
template <typename _Fct>
class final_action {
public:
  using function_type = _Fct;
  static_assert(!std::is_reference<_Fct>::value && !std::is_const<_Fct>::value && !std::is_volatile<_Fct>::value,
      "final_action should store its callable by value");

  explicit final_action(_Fct f) noexcept
      : _fct(std::move(f)) {}

  final_action(final_action&& other) noexcept
      : _fct(std::move(other._fct))
      , _need_invoke(std::exchange(other._need_invoke, false)) {}

  final_action(const final_action&) = delete;
  final_action& operator=(const final_action&) = delete;
  final_action& operator=(final_action&&) = delete;

  ~final_action() noexcept {
    if (_need_invoke) {
      _fct();
    }
  }

private:
  _Fct _fct;
  bool _need_invoke = true;
};

/// finally() - convenience function to generate a final_action.
// template <typename _Fct>
// FST_NODISCARD final_action<std::remove_cv_t<std::remove_reference_t<_Fct>>> finally(_Fct&& f) noexcept {
//  return final_action<std::remove_cv_t<std::remove_reference_t<_Fct>>>(std::forward<_Fct>(f));
//}
} // namespace fst.
