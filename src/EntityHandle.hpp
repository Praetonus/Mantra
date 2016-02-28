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

#ifndef MANTRA_ENTITYHANDLE_HPP
#define MANTRA_ENTITYHANDLE_HPP

#include "impl/Entity.hpp"

namespace mantra
{

/**
 * \brief Access point to an entity
 * 
 * References an entity and permits retrieval, addition and removal of components.
 * 
 * \tparam W Associated World type
 * \tparam P Primary component type
 * \tparam C Secondary component types
 * 
 * \note Instances are created and returned by the library and should not be created directly by the user.
 */
template <typename W, typename P, typename... C>
class EntityHandle final
#ifndef NDEBUG
//! \cond
	: public impl::DebugHandle<typename W::Components>
//! \endcond
#endif
{
	using WC = impl::WorldCont<typename W::Components, typename W::Systems>;

	public:
	//! \cond
	EntityHandle(typename WC::EntCont&, std::size_t);
	//! \endcond

	/**
	 * \brief `EntityHandle` is default copy constructible
	 */
	EntityHandle(EntityHandle const&) = default;
	/**
	 * \brief `EntityHandle` is not copy assignable
	 */
	EntityHandle& operator=(EntityHandle const&) = delete;
	
	/**
	 * \brief `EntityHandle` is default move constructible
	 * 
	 * \note The moved-from handle is invalidated
	 */
	EntityHandle(EntityHandle&&) = default;
	/**
	 * \brief `EntityHandle` is not move assignable
	 */
	EntityHandle& operator=(EntityHandle&&) = delete;

	/**
	 * \brief `EntityHandle` is default destructible
	 */
	~EntityHandle() = default;

	/**
	 * \brief Destroy the associated entity
	 * 
	 * \pre The handle is valid
	 * \note All associated handles are invalidated.
	 */
	void destroy();

#ifndef DOXYGEN_ONLY
	template <typename T>
	std::enable_if_t<impl::is_any<P, T, void>{}, T>& get_component() noexcept;

	template <typename T>
	std::enable_if_t<!std::is_pointer<T>{}, T> const& get_component() const noexcept;

	template <typename T>
	std::enable_if_t<std::is_pointer<T>{}, std::remove_pointer_t<T>> const* const&
		get_component() const noexcept;
#else
	/**
	 * \brief Retreive a component
	 * 
	 * \tparam T Type of the component
	 * \pre The handle is valid
	 * \return A reference to the component
	 * \note Only available if `T` is the primary component type or if there is no primary component.
	 */
	template <typename T>
	T& get_component() noexcept;
	
	/**
	 * \brief Retreive a component
	 * 
	 * \tparam T Type of the component
	 * \pre The handle is valid
	 * \return A constant reference to the component
	 * \note Only available if `T` is not a pointer type.
	 */
	template <typename T>
	T const& get_component() const noexcept;

	/**
	 * \brief Retreive a component
	 * 
	 * \tparam T Type of the component
	 * \pre The handle is valid
	 * \return A constant reference to the component. The pointer will see its pointed object as const.
	 * \note Only available if `T` is a pointer type.
	 */
	template <typename T>
	std::remove_pointer_t<T> const* const& get_component() const noexcept;
#endif // DOXYGEN_ONLY

	/**
	 * \brief Query the presence of components
	 * 
	 * \tparam Ts Types of the components
	 * \pre The handle is valid
	 * \return True if all `Ts` are present, false otherwise
	 */
	template <typename... Ts>
	bool has_components() const noexcept;

	/**
	 * \brief Add a component
	 * 
	 * \tparam T Type of the component
	 * \param args Parameters to construct the component
	 * \pre The handle is valid
	 * \pre The entity doesn't have the component
	 */
	template <typename T, typename... Args>
	void add_component(Args&&... args);
	
	/**
	 * \brief Add multiple components
	 * 
	 * \tparam Ts Types of the components
	 * \pre The handle is valid
	 * \pre The entity doesn't have the components
	 */
	template <typename... Ts>
	void add_components();

	/**
	 * \brief Add multiple components
	 * 
	 * \tparam Ts Types of the components
	 * \param args A pack of tuples holding the parameters to construct each component
	 * \pre The handle is valid
	 * \pre The entity doesn't have the components
	 */
	template <typename... Ts, typename... Args>
	void add_components(Args&&... args);

	/**
	 * \brief Remove components
	 * 
	 * \tparam Ts Types of the components
	 * \pre The handle is valid
	 * \pre The entity haves the components
	 */
	template <typename... Ts>
	void remove_components();

	/**
	 * \brief Equality comparison operator
	 * 
	 * \pre Both handles are valid
	 * \return True if both handles refer to the same entity, false otherwise
	 */
	friend bool operator==(mantra::EntityHandle<W, P, C...> const& l,
	                       mantra::EntityHandle<W, P, C...> const& r) noexcept
	{
		assert(l.valid_ && r.valid_ && "Entities aren't valid");

		return &l.entities_ == &r.entities_ && l.index_ == r.index_;
	}
	
	/**
	 * \brief Unequality comparison operator
	 * 
	 * \pre Both handle are valid
	 * \return True if the handles refer to distinct entities, false otherwise
	 */
	friend bool operator!=(mantra::EntityHandle<W, P, C...> const& l,
	                       mantra::EntityHandle<W, P, C...> const& r) noexcept
	{
		assert(l.valid_ && r.valid_ && "Entities aren't valid");

		return !(l == r);
	}

	private:
	typename WC::EntCont& entities_;
	std::size_t index_;
};

} // namespace mantra

#include "impl/EntityHandleImpl.hpp"

#endif // Header guard
