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
World<CL<C...>, SL<S...>>::World() noexcept
	: entities_{}, components_{}, systems_{}
{
	(void)impl::expand{(impl::validate_type_list(impl::TypeList<C...>{}, typename S::Components{}), 0)...};
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
	impl::validate_type_list(impl::TypeList<C...>{}, comp_types);

	auto it = std::find_if(std::begin(entities_), std::end(entities_),
	                       [](auto const& e){return !e;});
	if (it == std::end(entities_))
	{
		entities_.emplace_back(components_);
		it = std::end(entities_) - 1;
	}
	it->create(comp_types);
	return {entities_, static_cast<std::size_t>(std::distance(std::begin(entities_), it))};
}

template <typename... C, typename... S>
template <typename... Ts, typename... Args>
auto World<CL<C...>, SL<S...>>::create_entity(Args&&... args) -> EntityHandle<Self, void, C...>
{
	impl::TypeList<Ts...> comp_types{};
	impl::validate_type_list(impl::TypeList<C...>{}, comp_types);

	auto it = std::find_if(std::begin(entities_), std::end(entities_),
	                       [](auto const& e){return !e;});
	if (it == std::end(entities_))
	{
		entities_.emplace_back(components_);
		it = std::end(entities_) - 1;
	}
	it->create(comp_types, std::forward<Args>(args)...);
	return {entities_, static_cast<std::size_t>(std::distance(std::begin(entities_), it))};
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
template <typename T, typename P, typename... O>
void World<CL<C...>, SL<S...>>::update_(impl::TypeList<O...>)
{
	std::get<T>(systems_).update(WorldView<Self, P, O...>{entities_, components_});
}

} // namespace mantra

#endif // Header guard
