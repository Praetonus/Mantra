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

#ifndef MANTRA_WORLD_HPP
#define MANTRA_WORLD_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <tuple>
#include <vector>

#include "EntityHandle.hpp"
#include "impl/utility.hpp"

namespace mantra
{

template <typename... C>
using ComponentList = impl::TypeList<C...>;

template <typename... C>
using CL = ComponentList<C...>;

template <typename... S>
using SystemList = impl::TypeList<S...>;

template <typename... S>
using SL = SystemList<S...>;

template <typename Comp, typename Sys>
class World{private: World() {}};

template <typename... C, typename... S>
class World<CL<C...>, SL<S...>>
{
	public:
	World() noexcept;

	World(World const&) = delete;
	World& operator=(World const&) = delete;

	World(World&&) = default;
	World& operator=(World&&) = default;

	~World();

	template <typename... Ts>
	EntityHandle<C...> create_entity();

	private:
	std::vector<impl::Entity<C...>> entities_;
	std::tuple<std::vector<impl::Optional<C>>...> components_;
	std::tuple<S...> systems_;

	template <typename... U>
	void constexpr validate_type_list_(impl::TypeList<U...>);
};

template <typename... C, typename... S>
auto create_world(CL<C...>, SL<S...>)
{
	return World<CL<C...>, SL<S...>>{};
}

} // namespace mantra

#include "impl/World.hpp"

#endif // Header guard