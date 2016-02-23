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

#include "EntityHandle.hpp"
#include "tuple_create.hpp"

/**
 * \brief Library namespace
 */
namespace mantra
{

/**
 * \brief Helper type for component sets
 */
template <typename... C>
using ComponentList = impl::TypeList<C...>;

/**
 * \brief Helper type for system sets
 */
template <typename... S>
using SystemList = impl::TypeList<S...>;

//! \cond
template <typename... C>
using CL = ComponentList<C...>;

template <typename... S>
using SL = SystemList<S...>;

template <typename Comp, typename Sys>
class World;
//! \endcond

/**
 * \brief Main library class
 * 
 * The `World` holds entities and manages systems.
 * 
 * \tparam C The set of components types.
 * \tparam S The set of systems types.
 */
template <typename... C, typename... S>
class World<ComponentList<C...>, SystemList<S...>> final
{
	using Self = World<CL<C...>, SL<S...>>;
	public:
	//! \cond
	using Components = CL<C...>;
	using Systems = SL<S...>;
	//! \endcond
	
	/**
	 * \brief Constructor
	 * 
	 * Default-constructs the systems
	 * 
	 * \sa `create_world`
	 */
	World();

	/**
	 * \brief Constructor
	 * 
	 * Constructs the systems with `args`.
	 * 
	 * \param args A pack of tuples holding the parameters to construct each system
	 * \sa `create_world`
	 */
	template <typename... Args>
	World(Args&&... args);

	/**
	 * \brief `World` is not copy constructible
	 */
	World(World const&) = delete;
	/**
	 * \brief `World` is not copy assignable
	 */
	World& operator=(World const&) = delete;

	/**
	 * \brief `World` is default move constructible
	 */
	World(World&&) = default;
	/**
	 * \brief `World` is default move assignable
	 */
	World& operator=(World&&) = default;

	/**
	 * \brief Destructor
	 */
	~World();

	/**
	 * \brief Create a new entity
	 * 
	 * Default-constructs the components.
	 * 
	 * \tparam Ts Components the new entity will have
	 * \return An `EntityHandle` for the new entity
	 * \note The return type of this function is complex. It is advised to use automatic type deduction.
	 */
	template <typename... Ts>
	EntityHandle<Self, void, C...> create_entity();

	/**
	 * \brief Create a new entity
	 * 
	 * Constructs the components with `args`.
	 * 
	 * \tparam Ts Components the new entity will have
	 * \param args A pack of tuples holding the parameters to construct each component
	 * \return An `EntityHandle` for the new entity
	 * \note The return type of this function is complex. It is advised to use automatic type deduction.
	 */
	template <typename... Ts, typename... Args>
	EntityHandle<Self, void, C...> create_entity(Args&&... args);

	/**
	 * \brief Run a frame of the world
	 * 
	 * Each system is updated once.
	 * 
	 * \note The systems are updated sequentially, in the order in which they appear in `S`.
	 */
	void update();

	/**
	 * \brief Send a message to a system
	 * 
	 * Calls the system's `receive(A)` function.
	 * 
	 * \tparam T The type of the system
	 * \param arg The object to send
	 * \note The message is handled immediately in the same thread.
	 */
	template <typename T, typename A>
	void message(A&& arg);

	/**
	 * \brief Reserve space for future entities
	 * 
	 * \param n Number of entities to reserve. It is guaranteed that at least `n` new entities can be
	 * created without storage reallocation
	 */
	void reserve_entities(std::size_t n);

	/**
	 * \brief Reserve space for future components
	 *
	 * \tparam T Type of the components
	 * \param n Number of components to reserve. It is guaranteed that at least `n` new components can be
	 * created without storage reallocation
	 */
	template <typename T>
	void reserve_components(std::size_t n);

	private:
	template <typename T, typename P, typename... O>
	void update_(impl::TypeList<O...>);

	std::vector<impl::Entity<C...>> entities_;
	impl::Tuple<std::vector<boost::optional<C>>...> components_;
	impl::Tuple<S...> systems_;

	std::array<std::vector<std::size_t>, sizeof...(C) + 1> free_caches_;
};

/**
 * \brief Helper function to create a `World`
 *
 * \tparam C The set of components types.
 * \tparam S The set of systems types.
 * \param args A pack of tuples holding the parameters to construct each system
 */
template <typename... C, typename... S, typename... Args>
auto create_world(ComponentList<C...>, SystemList<S...>, Args&&... args)
{
	return World<CL<C...>, SL<S...>>{std::forward<Args>(args)...};
}

} // namespace mantra

#include "impl/World.hpp"

#endif // Header guard
