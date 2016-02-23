/*****
 * Copyright Benoit Vey (2016)
 *
 * benoit.vey@etu.upmc.fr
 *
 * This software is governed by the CeCILL-B license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL-B
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its terms.
 *****/

#ifndef MANTRA_IMPL_UTILITY_HPP
#define MANTRA_IMPL_UTILITY_HPP

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/optional.hpp>

namespace mantra
{

namespace impl
{

using expand = std::initializer_list<int>;

template <typename T, typename... Others>
struct is_any : std::false_type {};

template <typename T1, typename T2>
struct is_any<T1, T2> : std::is_same<T1, T2> {};

template <typename T, typename First, typename... Others>
struct is_any<T, First, Others...>
	: std::integral_constant<bool, std::is_same<T, First>{} || is_any<T, Others...>{}>
{};

template <typename, typename...>
struct IndexedTypeImpl;

template <std::size_t, typename>
struct IndexedTypeBase
{};

template <std::size_t... Is, typename... Ts>
struct IndexedTypeImpl<std::index_sequence<Is...>, Ts...> : IndexedTypeBase<Is, Ts>...
{};

template <typename... Ts>
using IndexedType = IndexedTypeImpl<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

template <typename T, std::size_t I>
std::size_t constexpr index_of_impl(IndexedTypeBase<I, T>) noexcept
{
	return I;
}

template <typename T, typename... Ts>
std::size_t constexpr index_of() noexcept
{
	return index_of_impl<T>(IndexedType<Ts...>{});
}

template <std::size_t I, typename T>
T type_of_impl(IndexedTypeBase<I, T>) noexcept;

template <std::size_t I, typename... Ts>
using TypeOf = decltype(type_of_impl<I>(IndexedType<Ts...>{}));

template <typename>
struct identity
{};

template <typename... Ts>
struct TypeList : private identity<Ts>... // Supplied same type multiple times
{
	static_assert(sizeof...(Ts) > 0, "No types supplied");

	template <typename T>
	bool constexpr contains() const noexcept
	{
		return std::is_base_of<identity<T>, TypeList>{};
	}
};

template <typename...>
struct Tuple;

template <typename T, typename... Ts>
constexpr T& get(Tuple<Ts...>&) noexcept;
template <typename T, typename... Ts>
constexpr T&& get(Tuple<Ts...>&&) noexcept;
template <typename T, typename... Ts>
constexpr T const& get(Tuple<Ts...> const&) noexcept;

struct piecewise_construct_t {};

piecewise_construct_t constexpr piecewise_construct{};

template <typename T>
struct TupleItem
{
	TupleItem() = default;
	template <typename U>
	constexpr TupleItem(U&& init) : item{std::forward<U>(init)} {}
	template <template <typename...> class Tp, typename... Us>
	constexpr TupleItem(piecewise_construct_t, Tp<Us...> init) : item{get<Us>(init)...} {}

	TupleItem(TupleItem const&) = default;
	TupleItem& operator=(TupleItem const&) = default;
	
	TupleItem(TupleItem&&) = default;
	TupleItem& operator=(TupleItem&&) = default;

	~TupleItem() = default;

	T item;
};

template <typename... Ts>
struct Tuple : public TupleItem<Ts>...
{
	Tuple() = default;
	template <typename... Us>
	constexpr Tuple(Us&&... init) : TupleItem<Ts>{std::forward<Us>(init)}... {}
	template <typename... Tups>
	constexpr Tuple(piecewise_construct_t pc, Tups&&... init)
		: TupleItem<Ts>{pc, std::forward<Tups>(init)}... {}

	Tuple(Tuple const&) = default;
	Tuple& operator=(Tuple const&) = default;

	Tuple(Tuple&&) = default;
	Tuple& operator=(Tuple&&) = default;

	~Tuple() = default;
};

template <typename T, typename... Ts>
constexpr T& get(Tuple<Ts...>& tuple) noexcept
{
	return static_cast<TupleItem<T>&>(tuple).item;
}

template <typename T, typename... Ts>
constexpr T&& get(Tuple<Ts...>&& tuple) noexcept
{
	return std::move(static_cast<TupleItem<T>&>(tuple).item);
}

template <typename T, typename... Ts>
constexpr T const& get(Tuple<Ts...> const& tuple) noexcept
{
	return static_cast<TupleItem<T> const&>(tuple).item;
}

template <std::size_t I, typename... Ts>
constexpr TypeOf<I, Ts...>& get(Tuple<Ts...>& tuple) noexcept
{
	return static_cast<TupleItem<TypeOf<I, Ts...>>&>(tuple).item;
}

template <std::size_t I, typename... Ts>
constexpr TypeOf<I, Ts...>&& get(Tuple<Ts...>&& tuple) noexcept
{
	return std::move(static_cast<TupleItem<TypeOf<I, Ts...>>&>(tuple).item);
}

template <std::size_t I, typename... Ts>
constexpr TypeOf<I, Ts...> const& get(Tuple<Ts...> const& tuple) noexcept
{
	return static_cast<TupleItem<TypeOf<I, Ts...>> const&>(tuple).item;
}

template <std::size_t, typename>
struct tuple_element;

template <typename T>
struct tuple_size;

template <typename... Ts>
struct tuple_size<Tuple<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <typename T>
struct tuple_size<T const> : public std::integral_constant<std::size_t, tuple_size<T>{}> {};

template <typename T>
struct tuple_size<T volatile> : public std::integral_constant<std::size_t, tuple_size<T>{}> {};

template <typename T>
struct tuple_size<T const volatile> : public std::integral_constant<std::size_t, tuple_size<T>{}> {};

template <typename F, typename T, std::size_t... I>
decltype(auto) invoke_helper(F&& f, T&& t, std::index_sequence<I...>)
{
	return f(get<I>(std::forward<T>(t))...);
}

template <typename F, typename T>
decltype(auto) invoke(F&& f, T&& t)
{
	auto constexpr S = tuple_size<std::decay_t<T>>{};
	return invoke_helper(std::forward<F>(f), std::forward<T>(t), std::make_index_sequence<S>{});
}

template <typename T, typename... C>
void constexpr validate_component(TypeList<C...> c) noexcept
{
	static_assert(c.template contains<T>(), "Invalid component type");
}

template <typename... C, typename... T>
void constexpr validate_components(TypeList<C...> c, TypeList<T...>) noexcept
{
	(void)expand
	{(
		validate_component<T, C...>(c), 0
	)...};
}

template <typename... Ts>
class Entity;

template <typename C, typename S>
struct WorldCont;

template <typename... C, typename... S>
struct WorldCont<TypeList<C...>, TypeList<S...>>
{
	using EntCont = std::vector<Entity<C...>>;
	using CompCont = Tuple<std::vector<boost::optional<C>>...>;
	using SysCont = Tuple<S...>;
	using Caches = std::array<std::vector<std::size_t>, sizeof...(C) + 1>;
};

} // namespace impl

} // namespace mantra

#endif // Header guard
