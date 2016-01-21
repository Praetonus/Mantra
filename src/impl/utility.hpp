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

#ifndef MANTRA_UTILITY_HPP
#define MANTRA_UTILITY_HPP

#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>

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
	: std::integral_constant<bool, std::is_same<T, First>::value || is_any<T, Others...>::value>
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
		return is_any<T, Ts...>::value;
	}

	private:
	template <typename Current, typename... Others>
	void constexpr validate_unicity_() noexcept
	{
		static_assert(!is_any<Current, Others...>::value, "Supplied same type multiple times");
		validate_unicity_<Others...>();
	}

	template <typename... Us>
	typename std::enable_if<sizeof...(Us) == 0>::type constexpr validate_unicity_() noexcept {}
};

template <typename T>
class Optional
{
	public:
	Optional() noexcept : exists_{false}
	{}

	Optional(Optional const&) = delete;
	Optional& operator=(Optional const&) = delete;

	Optional(Optional&& mv) noexcept(std::is_nothrow_move_constructible<T>::value) : exists_{mv.exists_}
	{
		if (exists_)
		{
			new (storage_) T{std::move(*mv.get())};
			mv.destroy();
		}
	}

	~Optional()
	{
		if (exists_)
			destroy();
	}

	template <typename... Args>
	void construct(Args&&... args)
	{
		assert(!exists_ && "Object is already existing");
		new (storage_) T{std::forward<Args>(args)...};
		exists_ = true;
	}

	void destroy() noexcept(std::is_nothrow_destructible<T>::value)
	{
		get()->~T();
		exists_ = false;
	}

	T* get() noexcept
	{
		assert(exists_ && "Object doesn't exists");
		return reinterpret_cast<T*>(storage_);
	}

	T const* get() const noexcept
	{
		assert(exists_ && "Object doesn't exists");
		return reinterpret_cast<T*>(storage_);
	}

	operator bool() const noexcept
	{
		return exists_;
	}

	bool operator!() const noexcept
	{
		return !exists_;
	}

	private:
	uint8_t storage_[sizeof(T)];
	bool exists_;
};

} // namespace impl

} // namespace mantra

#endif // Header guard