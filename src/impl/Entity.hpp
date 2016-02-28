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
	DebugHandle(std::vector<Entity<C...>>&, std::size_t);
	
	DebugHandle(DebugHandle const&);

	DebugHandle(DebugHandle&&);

	~DebugHandle();
	
	void invalidate_() noexcept;

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
	using Comps = impl::Tuple<std::vector<boost::optional<C>>...>;
	template <typename T>
	using CompVec = std::vector<boost::optional<T>>;
	using Caches = std::array<std::vector<std::size_t>, sizeof...(C) + 1>;

	public:
	Entity(Comps&, Caches&, std::size_t);

	Entity(Entity const&) = delete;
	Entity& operator=(Entity const&) = delete;

	Entity(Entity&&) noexcept;

	~Entity();

	template <typename... Ts>
	void create(TypeList<Ts...>);
	template <typename... Ts, typename... Args>
	void create(TypeList<Ts...>, Args&&...);

	void destroy();

	operator bool() const noexcept;
	bool operator!() const noexcept;

	template <typename T>
	T& get_component() noexcept;
	template <typename T>
	std::enable_if_t<!std::is_pointer<T>{}, T> const& get_component() const noexcept;
	template <typename P>
	std::enable_if_t<std::is_pointer<P>{}, std::remove_pointer_t<P>> const* const&
		get_pointer() const noexcept;

	template <typename... Ts>
	bool has_components() const noexcept;

	template <typename T, typename... Args>
	void add_component(Args&&...);
	template <typename... Ts>
	void add_components();
	template <typename... Ts, typename... Args>
	void add_components(Args&&...);
	
	template <typename... Ts>
	void remove_components();
	
#ifndef NDEBUG
	void add_handle(DebugHandle<TypeList<C...>>*);
	void remove_handle(DebugHandle<TypeList<C...>>*);
	void invalidate_handles() noexcept;
#endif // NDEBUG

	private:
	template <typename T, typename Tuple, typename... Ts>
	void assign_comp_(CompVec<T>&, Tuple&&, TypeList<Ts...>);

	Comps& comps_;
	std::array<long, sizeof...(C)> comps_idx_;
	Caches& free_caches_;
#ifndef NDEBUG
	std::vector<DebugHandle<TypeList<C...>>*> handles_;
#endif // NDEBUG
	std::size_t index_;
	bool exists_;
};

} // namespace impl

} // namespace mantra

#include "EntityImpl.hpp"

#endif // Header guard
