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

#ifndef MANTRA_SYSTEM_HPP
#define MANTRA_SYSTEM_HPP

#include <vector>

#include "WorldView.hpp"
#include "tuple_create.hpp"

namespace mantra
{

/**
 * \brief Helper class for systems
 * 
 * You can inherit from this class to create systems. Required components are specified in the template
 * parameters. The logic of the system should be implemented by redefinig the `update` function.
 * 
 * \tparam P Primary component type. The primary component is the only writable component.
 * If `P` is void, there is no primary component and the system can't write to anything
 * \tparam C Secondary components types. Secondary components are read-only. The system will only be able to
 * access entities that possess all secondary components and the primary component, if any
 * \note Inheriting from `System` is a convenience, but it is not mandatory. You can create your own isolated
 * class and provide
 * \arg A type named `Primary`, which can be `void`
 * \arg A type named `Components` defined as `ComponentList<C...>`, with `C` the list of components types
 * which should include `Primary` if it is not `void`, and should not include it if it is
 * \arg A function `void %update(WV&&)` with `WV` template
 */
template <typename P, typename... C>
class System
{
	public:
	//! \cond
	using Primary = P;
	using Components = std::conditional_t<std::is_same<P, void>{},
	                                      impl::TypeList<C...>, impl::TypeList<P, C...>>;
	//! \endcond

	/**
	 * \brief `System` is default constructible
	 */
	System() = default;
	
	/**
	 * \brief `System` is default copy constructible
	 */
	System(System const&) = default;
	/**
	 * \brief `System` is default copy assignable
	 */
	System& operator=(System const&) = default;

	/**
	 * \brief `System` is default move constructible
	 */
	System(System&&) = default;
	/**
	 * \brief `System` is default move assignable
	 */
	System& operator=(System&&) = default;

	/**
	 * \brief Update function
	 * 
	 * Redefine this in your system class. The function will be called by the `World` on each frame.
	 * 
	 * \param wv A `WorldView` corresponding to the system's component types. This is template to allow the
	 * use of the system with different `World` types
	 */
	template <typename WV>
	void update(WV&& wv);

#ifdef DOXYGEN_ONLY
	//! \cond
	using Type = int;
	//! \endcond

	/**
	 * \brief Receive messages
	 * 
	 * Define this function to handle messages sent to this system. `Type` can be any type and you can define
	 * as much overloads as you need (one per message type).
	 */
	void receive(Type);
#endif

	protected:
	/**
	 * \brief
	 * `System` is default destructible
	 */
	~System() = default;
};

} // namespace mantra

#endif // Header guard
