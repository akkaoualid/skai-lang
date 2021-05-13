#ifndef SKAI_UTILS_HPP_739393948
#define SKAI_UTILS_HPP_739393948
namespace skai {
template <class T, class U>
T as(U&& arg) {
    return static_cast<T>(std::forward<U>(arg));
}
}  // namespace skai
#endif
