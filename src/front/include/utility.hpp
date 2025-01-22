#pragma once
#include <variant>
#include <type_traits>
#include <memory>

namespace tinyc
{
namespace util
{

/**
 * @brief 确保所有的RestType调用Func的返回值相同
 */
template<typename Func, typename Type,  typename... RestTypes>
struct invoke_result_are_same
{
	using type = std::invoke_result_t<Func, Type>;

	inline static constexpr bool value = invoke_result_are_same<Func, RestTypes...>::value &&
		std::is_same_v<type, typename invoke_result_are_same<Func, RestTypes...>::type>;
};


template<typename Func, typename LeftType, typename RightType>
struct invoke_result_are_same<Func, LeftType, RightType>
{
	using type = std::invoke_result_t<Func, LeftType>;

	inline static constexpr bool value = std::is_same_v<
		std::invoke_result_t<Func, LeftType>, std::invoke_result_t<Func, RightType>
	>;

	static_assert(value, "Return types of Func invoked with types are not consistent.");
};



template<typename Func, typename Type>
struct invoke_result_are_same<Func, Type>
{
	using type = std::invoke_result_t<Func, Type>;
	inline static constexpr bool value = true;
};


/**
 * @brief std::unique_ptr<Ty> 关于Ty的类型萃取
 */
template<typename Ty>
struct uptr_elem
{
	static_assert(false, "uptr_elem template type must be wrappered by std::unique_ptr");
};


template<typename Ty>
struct uptr_elem<std::unique_ptr<Ty>>
{
	using type = Ty;
};


template<typename Uptr>
using uptr_elem_t = typename uptr_elem<Uptr>::type;


/**
 * @brief 对于std::variant, 获取其visit的返回类型
 */
template<typename Variant>
struct variant_types
{
	static_assert(false,
				  "Invalid instantiation with expected type, not std::variant");
};


template<typename... Types>
struct variant_types<std::variant<Types...>>
{
	template<typename Func>
	using visit_result_type = typename invoke_result_are_same<Func, Types...>::type;

 	/// @brief 对于内部为unique_ptr的variant, 获取visit的回调函数参数为unique_ptr的元素的特殊处理
	template<typename Func>
	using nf_visit_result_type = typename invoke_result_are_same<Func, uptr_elem_t<Types>...>::type;
	
};


template <typename Func, typename Variant>
using visit_result_t = typename variant_types<
	std::decay_t<Variant>>::template visit_result_type<std::decay_t<Func>>;


template <typename NormalFunc, typename RowVariant>
using nf_visit_result_t = typename variant_types<
	std::decay_t<RowVariant>>::template nf_visit_result_type<std::decay_t<NormalFunc>>;


template<typename NormalFunc, typename RowVariant>
struct variant_uptr_deref_func
{
	template<typename Arg>
	auto operator()(Arg&& arg) -> nf_visit_result_t<NormalFunc, RowVariant>
	{
		return NormalFunc{}(*std::forward<Arg>(arg));
	};
};

//template<typename Func, typename... ArgType>
//struct uptr_deref_func
//{
//	static
//	auto func(Func&& func, ArgType&&... args)
//	{
//		return std::forward<Func>(func)(std::forward<ArgType>(*args)...);
//	}
//
//	using type = decltype(func);
//};
//
//
//template <typename Func, typename Variant>
//using uptr_store_visit_ret_t =
//	visit_result_t<typename uptr_deref_func<Func, Variant>::type, Variant>;

}	//namespace tinyc::util
}	//namespace tinyc

