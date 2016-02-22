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


#ifndef MANTRA_IMPL_WORLD_HPP
#define MANTRA_IMPL_WORLD_HPP

#include "../World.hpp"

#include "../WorldView.hpp"

namespace mantra
{

template <typename... C, typename... S>
World<CL<C...>, SL<S...>>::World()
	: entities_{}, components_{}, systems_{}, free_caches_{}
{
	(void)impl::expand{(impl::validate_components(impl::TypeList<C...>{}, typename S::Components{}), 0)...};
}

template <typename... C, typename... S>
template <typename... Args>
World<CL<C...>, SL<S...>>::World(Args&&... args)
	: entities_{}, components_{}, systems_{impl::construct<S>(std::forward<Args>(args))...}, free_caches_{}
{
	(void)impl::expand{(impl::validate_components(impl::TypeList<C...>{}, typename S::Components{}), 0)...};
}

template <typename... C, typename... S>
World<CL<C...>, SL<S...>>::~World()
{
	entities_.clear();
}

template <typename... C, typename... S>
template <typename... Ts>
auto World<CL<C...>, SL<S...>>::create_entity() -> EntityHandle<Self, void, C...>
{
	impl::TypeList<Ts...> comp_types{};
	impl::validate_components(impl::TypeList<C...>{}, comp_types);

	auto it = std::begin(entities_);
	if (!free_caches_[0].empty())
	{
		auto index = free_caches_[0].back();
		free_caches_[0].pop_back();
		it += static_cast<std::ptrdiff_t>(index);
	}
	else
	{
		it = std::find_if(std::begin(entities_), std::end(entities_),
		                  [](auto const& e){return !e;});
		if (it == std::end(entities_))
		{
			entities_.emplace_back(components_, free_caches_, entities_.size());
			it = std::end(entities_) - 1;
		}
	}
	it->create(comp_types);
	return {entities_, static_cast<std::size_t>(it - std::begin(entities_))};
}

template <typename... C, typename... S>
template <typename... Ts, typename... Args>
auto World<CL<C...>, SL<S...>>::create_entity(Args&&... args) -> EntityHandle<Self, void, C...>
{
	impl::TypeList<Ts...> comp_types{};
	impl::validate_components(impl::TypeList<C...>{}, comp_types);

	auto it = std::begin(entities_);
	if (!free_caches_[0].empty())
	{
		auto index = free_caches_[0].back();
		free_caches_[0].pop_back();
		it += static_cast<std::ptrdiff_t>(index);
	}
	else
	{
		it = std::find_if(std::begin(entities_), std::end(entities_),
		                  [](auto const& e){return !e;});
		if (it == std::end(entities_))
		{
			entities_.emplace_back(components_, free_caches_, entities_.size());
			it = std::end(entities_) - 1;
		}
	}
	it->create(comp_types, std::forward<Args>(args)...);
	return {entities_, static_cast<std::size_t>(it - std::begin(entities_))};
}

template <typename... C, typename... S>
void World<CL<C...>, SL<S...>>::update()
{
	(void)impl::expand
	{(
		update_<S, typename S::Primary>(typename S::Components{}), 0
	)...};
}

template <typename... C, typename... S>
template <typename T, typename A>
void World<CL<C...>, SL<S...>>::message(A&& arg)
{
	std::get<T>(systems_).receive(std::forward<A>(arg));
}

template <typename... C, typename... S>
void World<CL<C...>, SL<S...>>::reserve_entities(std::size_t n)
{
	if (free_caches_[0].size() < n)
		entities_.reserve(entities_.size() + n - free_caches_[0].size());
}

template <typename... C, typename... S>
template <typename T>
void World<CL<C...>, SL<S...>>::reserve_components(std::size_t n)
{
	impl::validate_component<T>(impl::TypeList<C...>{});

	auto& cache = free_caches_[impl::TypeToIndex<1, T, C...>()];
	if (cache.size() < n)
	{
		auto& comps = std::get<std::vector<boost::optional<T>>>(components_);
		comps.reserve(comps.size() + n - cache.size());
	}
}

template <typename... C, typename... S>
template <typename T, typename P, typename... O>
void World<CL<C...>, SL<S...>>::update_(impl::TypeList<O...>)
{
	using TP = std::conditional_t<std::is_same<P, void>{}, void const, P>;
	std::get<T>(systems_).update(WorldView<Self, TP, O...>{entities_, components_, systems_, free_caches_});
}

} // namespace mantra

#endif // Header guard
