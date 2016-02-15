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

#ifndef MANTRA_IMPL_ENTITY_HPP
#define MANTRA_IMPL_ENTITY_HPP

#include <array>
#include <cassert>
#include <functional>
#include <vector>

#include <boost/optional.hpp>

#include "utility.hpp"

namespace mantra
{

namespace impl
{

class EntityKey;

#ifndef NDEBUG
template <typename... C>
class Entity;

template <typename C>
class DebugHandle;

template <typename... C>
class DebugHandle<TypeList<C...>>
{
	public:
	DebugHandle(std::vector<Entity<C...>>& entities, std::size_t index)
		: entities_{entities}, index_{index}, valid_{true}
	{
		entities_[index_].add_handle(this);
	}
	
	DebugHandle(DebugHandle const& cp)
		: entities_{cp.entities_}, index_{cp.index_}, valid_{cp.valid_}
	{
		if (valid_)
			entities_[index_].add_handle(this);
	}

	DebugHandle(DebugHandle&& mv)
		: entities_{mv.entities_}, index_{mv.index_}, valid_{mv.valid_}
	{
		if (valid_)
		{
			entities_[index_].remove_handle(&mv);
			mv.valid_ = false;
			entities_[index_].add_handle(this);
		}
	}

	~DebugHandle()
	{
		if (valid_)
		{
			entities_[index_].remove_handle(this);
			valid_ = false;
		}
	}
	
	void invalidate_() noexcept
	{
		valid_ = false;
	}

	private:
	std::vector<Entity<C...>>& entities_;
	std::size_t index_;

	protected:
	bool valid_;
};

#endif // NDEBUG

template <typename... C>
class Entity
{
	using Comps = std::tuple<std::vector<boost::optional<C>>...>;
	template <typename T>
	using CompVec = std::vector<boost::optional<T>>;

	template <std::size_t I, typename T, typename... Cs>
	struct TypeToIndex : std::integral_constant<std::size_t, 0>
	{};

	template <std::size_t I, typename T, typename U, typename... Cs>
	struct TypeToIndex<I, T, U, Cs...> : std::integral_constant<std::size_t, TypeToIndex<I+1, T, Cs...>{}>
	{};

	template <std::size_t I, typename T, typename... Cs>
	struct TypeToIndex<I, T, T, Cs...> : std::integral_constant<std::size_t, I>
	{};

	public:
	Entity(Comps& comps) : comps_{comps}, comps_idx_{},
#ifndef NDEBUG
	handles_{},
#endif // NDEBUG
	exists_{false}
	{
		comps_idx_.fill(-1);
	}

	Entity(Entity const&) = delete;
	Entity& operator=(Entity const&) = delete;

	Entity(Entity&& mv) noexcept : comps_{mv.comps_}, comps_idx_{std::move(mv.comps_idx_)},
#ifndef NDEBUG
	handles_{std::move(mv.handles_)},
#endif // NDEBUG
	exists_{mv.exists_}
	{
		mv.exists_ = false;
	}

	~Entity()
	{
		if (exists_)
			destroy();
	}

	template <typename... Ts>
	void create(TypeList<Ts...> types)
	{
		assert(!exists_ && "Entity already exists");
		
		(void)expand
		{(
			assign_comp_(std::get<CompVec<Ts>>(comps_), std::tuple<>{}, types)
			, 0
		)...};
		exists_ = true;
	}

	template <typename... Ts, typename... Args>
	void create(TypeList<Ts...> types, Args&&... args)
	{
		assert(!exists_ && "Entity already exists");
	
		(void)expand
		{(
			assign_comp_(std::get<CompVec<Ts>>(comps_), std::forward<Args>(args), types), 0
		)...};
		exists_ = true;
	}

	void destroy()
	{
		assert(exists_ && "Entity doesn't exists");
#ifndef NDEBUG	
		invalidate_handles();
#endif

		auto it = std::begin(comps_idx_);
		(void)expand
		{(
			*it != -1 ? (void)(std::get<CompVec<C>>(comps_)[static_cast<std::size_t>(*it++)] = boost::none)
			          : (void)++it, 0
		)...};
		std::fill(std::begin(comps_idx_), std::end(comps_idx_), -1);
		exists_ = false;
	}

	operator bool() const noexcept
	{
		return exists_;
	}

	bool operator!() const noexcept
	{
		return !exists_;
	}

	template <typename T>
	T& get_component() noexcept
	{
		auto idx = comps_idx_[TypeToIndex<0, T, C...>()];
		assert(exists_ && "Entity doesn't exists");
		assert((idx != -1) && "Entity doesn't have this component");
		
		return std::get<CompVec<T>>(comps_)[static_cast<std::size_t>(idx)].get();
	}
	
	template <typename T>
	std::enable_if_t<!std::is_pointer<T>{}, T> const& get_component() const noexcept
	{
		auto idx = comps_idx_[TypeToIndex<0, T, C...>()];
		assert(exists_ && "Entity doesn't exists");
		assert((idx != -1) && "Entity doesn't have this component");

		return std::get<CompVec<T>>(comps_)[static_cast<std::size_t>(idx)].get();
	}

	template <typename P>
	std::enable_if_t<std::is_pointer<P>{}, std::remove_pointer_t<P>> const* const&
		get_pointer() const noexcept
	{
		auto idx = comps_idx_[TypeToIndex<0, P, C...>()];
		assert(exists_ && "Entity doesn't exists");
		assert((idx != -1) && "Entity doesn't have this component");

		using T = std::remove_pointer_t<P>;

		return *const_cast<T const**>(&std::get<CompVec<P>>(comps_)[static_cast<std::size_t>(idx)].get());
	}

	template <typename... Ts>
	bool has_components() const noexcept
	{
		
		for (auto i : {TypeToIndex<0, Ts, C...>::value...})
		{
			if (comps_idx_[i] == -1)
				return false;
		}
		return true;
	}

	template <typename T, typename... Args>
	void add_component(Args&&... args)
	{
		assert(exists_ && "Entity doesn't exists");
#ifndef NDEBUG	
		assert((comps_idx_[TypeToIndex<0, T, C...>()] == -1) && "Entity already has this component");
#endif
		
		assign_comp_(std::get<CompVec<T>>(comps_), std::forward_as_tuple(std::forward<Args>(args)...),
		             TypeList<T>{});
	}

	template <typename... Ts>
	void add_components()
	{
		assert(exists_ && "Entity doesn't exists");
#ifndef NDEBUG	
		(void)expand
		{(
			assert((comps_idx_[TypeToIndex<0, Ts, C...>()] == -1) && "Entity already has this component"), 0
		)...};
#endif
		(void)expand
		{(
			assign_comp_(std::get<CompVec<Ts>>(comps_), std::tuple<>{}, TypeList<Ts...>{})
			, 0
		)...};
	}

	template <typename... Ts, typename... Args>
	void add_components(Args&&... args)
	{
		assert(exists_ && "Entity doesn't exists");
#ifndef NDEBUG	
		(void)expand
		{(
			assert((comps_idx_[TypeToIndex<0, Ts, C...>()] == -1) && "Entity already has this component"), 0
		)...};
#endif
		
		(void)expand
		{(
			assign_comp_(std::get<CompVec<Ts>>(comps_), std::forward<Args>(args), TypeList<Ts...>{}), 0
		)...};
	}

	template <typename... Ts>
	void remove_components()
	{
		assert(exists_ && "Entity doesn't exists");
#ifndef NDEBUG
		(void)expand
		{(
			assert((comps_idx_[TypeToIndex<0, Ts, C...>()] != -1) && "Entity doesn't have this component"), 0
		)...};
#endif
		
		(void)expand
		{(
			std::get<CompVec<Ts>>(comps_)
				[static_cast<std::size_t>(comps_idx_[TypeToIndex<0, Ts, C...>()])] = boost::none,
			comps_idx_[TypeToIndex<0, Ts, C...>()] = -1, 0
		)...};
	}
	
#ifndef NDEBUG
	void add_handle(DebugHandle<TypeList<C...>>* handle)
	{
		handles_.emplace_back(handle);
	}

	void remove_handle(DebugHandle<TypeList<C...>>* handle)
	{
		handles_.erase(std::remove(std::begin(handles_), std::end(handles_), handle), std::end(handles_));
	}

	void invalidate_handles() noexcept
	{
		for (auto elem : handles_)
			elem->invalidate_();
		handles_.clear();
	}
#endif // NDEBUG

	private:
	template <typename T, typename Tuple, typename... Ts>
	void assign_comp_(CompVec<T>& comps, Tuple&& args, TypeList<Ts...>)
	{
		auto it = std::find_if(std::begin(comps), std::end(comps),
		                       [](auto const& e){return !e;});
		if (it == std::end(comps))
		{
			comps.emplace_back();
			it = std::end(comps) - 1;
		}
		invoke([it](auto&&... a){it->emplace(std::forward<decltype(a)>(a)...);},
		       std::forward<Tuple>(args));
		comps_idx_[TypeToIndex<0, T, C...>()] = std::distance(std::begin(comps), it);
	}

	Comps& comps_;
	std::array<long, sizeof...(C)> comps_idx_;
#ifndef NDEBUG
	std::vector<DebugHandle<TypeList<C...>>*> handles_;
#endif // NDEBUG
	bool exists_;
};

} // namespace impl

} // namespace mantra

#endif // Header guard
