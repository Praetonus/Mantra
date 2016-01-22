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
#include <vector>
#include <typeinfo>

#include "utility.hpp"

namespace mantra
{

template <typename... C>
class EntityHandle;

namespace impl
{

class EntityKey
{
	template <typename... C>
	friend class Entity;
	template <typename... C>
	friend class EntityHandle;
};

template <typename... C>
class Entity
{
	using Comps = std::tuple<std::vector<Optional<C>>...>;
	template <typename T>
	using CompVec = std::vector<Optional<T>>;

	template <std::size_t I, typename T, typename U, typename... Cs>
	struct TypeToIndex : std::integral_constant<std::size_t, TypeToIndex<I+1, T, Cs...>::value>
	{};

	template <std::size_t I, typename T, typename... Cs>
	struct TypeToIndex<I, T, T, Cs...> : std::integral_constant<std::size_t, I>
	{};

	public:
	Entity(Comps& comps) : comps_{comps}, comps_idx_{{-1}},
#ifndef NDEBUG
	handles_{},
#endif // NDEBUG
	exists_{false}
	{}

	Entity(Entity const&) = delete;
	Entity& operator=(Entity const&) = delete;

	Entity(Entity&& mv) : comps_{mv.comps_}, comps_idx_{std::move(mv.comps_idx_)},
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
	void create(TypeList<Ts...>)
	{
		assert(!exists_ && "Entity already exists");
		
		(void)expand
		{(
			assign_comp_<C>(std::get<CompVec<C>>(comps_), is_any<C, Ts...>::value), 0
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
			*it != -1 ? std::get<CompVec<C>>(comps_)[static_cast<std::size_t>(*it++)].destroy()
			          : (void)++it, 0
		)...};
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
		auto idx = comps_idx_[TypeToIndex<0, T, C...>::value];
		assert(exists_ && "Entity doesn't exists");
		assert((idx != -1) && "Entity doesn't have this component");
		
		return *std::get<CompVec<T>>(comps_)[static_cast<std::size_t>(idx)].get();
	}
	
	template <typename T>
	T const& get_component() const noexcept
	{
		auto idx = comps_idx_[TypeToIndex<0, T, C...>::value];
		assert(exists_ && "Entity doesn't exists");
		assert((idx != -1) && "Entity doesn't have this component");

		return *std::get<CompVec<T>>(comps_)[static_cast<std::size_t>(idx)].get();
	}

	template <typename... Ts>
	void add_components()
	{
		assert(exists_ && "Entity doesn't exists");
#ifndef NDEBUG	
		(void)expand
		{(
			assert((comps_idx_[TypeToIndex<0, Ts, C...>::value] == -1) && "Entity already has this component"), 0
		)...};
#endif
		
		(void)expand
		{(
			assign_comp_<Ts>(std::get<CompVec<Ts>>(comps_), true), 0
		)...};
	}

	template <typename... Ts>
	void remove_components()
	{
		assert(exists_ && "Entity doesn't exists");
#ifndef NDEBUG
		(void)expand
		{(
			assert((comps_idx_[TypeToIndex<0, Ts, C...>::value] != -1) && "Entity doesn't have this component"), 0
		)...};
#endif
		
		(void)expand
		{(
			std::get<CompVec<Ts>>(comps_)
				[static_cast<std::size_t>(comps_idx_[TypeToIndex<0, Ts, C...>::value])].destroy(),
			comps_idx_[TypeToIndex<0, Ts, C...>::value] = -1, 0
		)...};
	}
	
#ifndef NDEBUG
	void add_handle(EntityHandle<C...>* handle)
	{
		handles_.emplace_back(handle);
	}

	void remove_handle(EntityHandle<C...>* handle)
	{
		handles_.erase(std::remove(std::begin(handles_), std::end(handles_), handle), std::end(handles_));
	}

	void invalidate_handles()
	{
		for (auto elem : handles_)
			elem->invalidate_(EntityKey{});
		handles_.clear();
	}
#endif // NDEBUG

	private:
	template <typename T>
	void assign_comp_(std::vector<Optional<T>>& comps, bool assign)
	{
		if (assign)
		{
			auto it = std::find_if(std::begin(comps), std::end(comps),
			                       [](auto const& e){return !e;});
			if (it == std::end(comps))
			{
				comps.emplace_back();
				it = std::end(comps) - 1;
			}
			it->construct();
			comps_idx_[TypeToIndex<0, T, C...>::value] = std::distance(std::begin(comps), it);
		}
		else
			comps_idx_[TypeToIndex<0, T, C...>::value] = -1;
	}

	Comps& comps_;
	std::array<long, sizeof...(C)> comps_idx_;
#ifndef NDEBUG
	std::vector<EntityHandle<C...>*> handles_;
#endif // NDEBUG
	bool exists_;
};

} // namespace impl

} // namespace mantra

#endif // Header guard
