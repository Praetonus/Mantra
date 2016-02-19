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

using expand = int[];

template <typename T, typename... Others>
struct is_any : std::false_type {};

template <typename T1, typename T2>
struct is_any<T1, T2> : std::is_same<T1, T2> {};

template <typename T, typename First, typename... Others>
struct is_any<T, First, Others...>
	: std::integral_constant<bool, std::is_same<T, First>{} || is_any<T, Others...>{}>
{};

template <typename... Ts>
struct TypeList
{
	static_assert(sizeof...(Ts) > 0, "No types supplied");

	constexpr TypeList() noexcept
	{
		validate_unicity_<Ts...>();
	}

	template <typename T>
	bool constexpr contains() const noexcept
	{
		return is_any<T, Ts...>{};
	}

	private:
	template <typename Current, typename... Others>
	void constexpr validate_unicity_() noexcept
	{
		static_assert(!is_any<Current, Others...>{}, "Supplied same type multiple times");
		validate_unicity_<Others...>();
	}

	template <typename... Us>
	typename std::enable_if<sizeof...(Us) == 0>::type constexpr validate_unicity_() noexcept {}
};

template <typename F, typename T, std::size_t... I>
decltype(auto) invoke_helper(F&& f, T&& t, std::index_sequence<I...>)
{
	return f(std::get<I>(std::forward<T>(t))...);
}

template <typename F, typename T>
decltype(auto) invoke(F&& f, T&& t)
{
	auto constexpr S = std::tuple_size<typename std::decay<T>::type>{};
	return invoke_helper(std::forward<F>(f), std::forward<T>(t), std::make_index_sequence<S>{});
}

template <typename T, typename... C>
void constexpr validate_component(TypeList<C...>) noexcept
{
	static_assert(is_any<T, C...>{}, "Invalid component type");
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
	using CompCont = std::tuple<std::vector<boost::optional<C>>...>;
	using SysCont = std::tuple<S...>;
};

} // namespace impl

} // namespace mantra

#endif // Header guard
