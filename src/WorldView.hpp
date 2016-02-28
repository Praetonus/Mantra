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

#ifndef MANTRA_WORLDVIEW_HPP
#define MANTRA_WORLDVIEW_HPP

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

/**
 * \brief View of a World from a system
 * 
 * \tparam W Associated `World` type
 * \tparam P Primary component type
 * \tparam C Secondary component types
 * 
 * \note Instances are created and returned by the library and should not be created directly by the user.
 */
template <typename W, typename P, typename... C>
class WorldView final
{
	using WC = impl::WorldCont<typename W::Components, typename W::Systems>;

	class EntityIterator : public std::iterator<
	                                std::forward_iterator_tag,
	                                EntityHandle<W, P, C...>,
	                                std::ptrdiff_t,
	                                EntityHandle<W, P, C...>*,
	                                EntityHandle<W, P, C...>&>
	{
		public:
		EntityIterator();
		explicit EntityIterator(WorldView<W, P, C...>&);
		
		EntityIterator(EntityIterator const&);

		EntityHandle<W, P, C...>& operator*();
		EntityHandle<W, P, C...>* operator->();

		EntityIterator& operator++();
		EntityIterator operator++(int);

		friend bool operator==(typename mantra::WorldView<W, P, C...>::EntityIterator const& l,
		                       typename mantra::WorldView<W, P, C...>::EntityIterator const& r) noexcept
		{
			return l.view_ == r.view_ && l.index_ == r.index_;
		}
		
		friend bool operator!=(typename mantra::WorldView<W, P, C...>::EntityIterator const& l,
		                       typename mantra::WorldView<W, P, C...>::EntityIterator const& r) noexcept
		{
			return !(l == r);
		}

		private:
		void find_next_();

		WorldView<W, P, C...>* view_;
		boost::optional<EntityHandle<W, P, C...>> handle_;
		std::size_t index_;
	};

	class Entities
	{
		public:
		explicit Entities(WorldView<W, P, C...>&);

		EntityIterator begin();
		EntityIterator end();

		private:
		WorldView<W, P, C...>& view_;
	};

	public:
	//! \cond
	WorldView(typename WC::EntCont&, typename WC::CompCont&, typename WC::SysCont&,
	          typename WC::Caches&) noexcept;
	//! \endcond

	/**
	 * \brief `WorldView` is not copy constructible
	 */
	WorldView(WorldView const&) = delete;
	/**
	 * \brief `WorldView` is not copy assignable
	 */
	WorldView& operator=(WorldView const&) = delete;
	
	/**
	 * \brief `WorldView` is not move constructible
	 */
	WorldView(WorldView&&) = delete;
	/**
	 * \brief `WorldView` is not move assignable
	 */
	WorldView& operator=(WorldView&&) = delete;

	/**
	 * \brief `WorldView` is default destructible
	 */
	~WorldView() = default;

	/**
	 * \brief Create a new entity
	 * 
	 * Default-constructs the components
	 * 
	 * \tparam Ts Components the new entity will have
	 * \return An `EntityHandle` for the new entity
	 * \note The return type of this function is complex. It is advised to use automatic type deduction.
	 */
	template <typename... Ts>
	EntityHandle<W, P, C...> create_entity();

	/**
	 * \brief Create a new entity
	 * 
	 * Constructs the components with args
	 * 
	 * \tparam Ts Components the new entity will have
	 * \param args A pack of tuples holding the parameters to construct each component
	 * \return An `EntityHandle` for the new entity
	 * \note The return type of this function is complex. It is advised to use automatic type deduction.
	 */
	template <typename... Ts, typename... Args>
	EntityHandle<W, P, C...> create_entity(Args&&... args);

	/**
	 * \brief Iterator interface for entities
	 * 
	 * \return An object with `begin()` and `end()` functions, which can be used to obtain iterators over
	 * the entities visible by this `WorldView`, via `EntityHandle`s.
	 * \note The iterators meet the requirements of `ForwardIterator`.
	 * \note If an entity is destroyed, iterators referencing this entity are invalidated.
	 */
	Entities entities();

	/**
	 * \brief Send a message to a system
	 * 
	 * Calls the system's `receive(A)` function.
	 * 
	 * \tparam S The type of the system
	 * \param arg The object to send
	 * \note The message is handled immediately in the same thread.
	 */
	template <typename S, typename A>
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
	typename WC::EntCont& entities_;
	typename WC::CompCont& components_;
	typename WC::SysCont& systems_;
	typename WC::Caches& free_caches_;
};

} // namespace mantra

#include "impl/WorldViewImpl.hpp"

#endif // Header guard
