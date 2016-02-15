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
class World final {private: World() {}};

template <typename... C, typename... S>
class World<CL<C...>, SL<S...>> final
{
	using Self = World<CL<C...>, SL<S...>>;
	public:
	using Components = CL<C...>;
	using Systems = SL<S...>;
	
	World() noexcept;

	World(World const&) = delete;
	World& operator=(World const&) = delete;

	World(World&&) = default;
	World& operator=(World&&) = default;

	~World();

	template <typename... Ts>
	EntityHandle<Self, void, C...> create_entity();

	template <typename... Ts, typename... Args>
	EntityHandle<Self, void, C...> create_entity(Args&&...);

	void update();

	private:
	template <typename T, typename P, typename... O>
	void update_(impl::TypeList<O...>);

	std::vector<impl::Entity<C...>> entities_;
	std::tuple<std::vector<boost::optional<C>>...> components_;
	std::tuple<S...> systems_;
};

template <typename... C, typename... S>
auto create_world(CL<C...>, SL<S...>)
{
	return World<CL<C...>, SL<S...>>{};
}

} // namespace mantra

#include "impl/World.hpp"

#endif // Header guard
