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

#ifndef MANTRA_IMPL_ENTITYHANDLE_HPP
#define MANTRA_IMPL_ENTITYHANDLE_HPP

#include "../EntityHandle.hpp"

namespace mantra
{

template <typename P, typename... C>
EntityHandle<P, C...>::EntityHandle(std::vector<impl::Entity<C...>>& entities, std::size_t index)
	: 
#ifndef NDEBUG
	  impl::DebugHandle<C...>{entities, index},
#endif
	  entities_{entities}, index_{index}
{}

template <typename P, typename... C>
void EntityHandle<P, C...>::destroy()
{
	assert(this->valid_ && "Entity isn't valid");

	entities_[index_].destroy();
}

template <typename P, typename... C>
template <typename T>
std::enable_if_t<impl::is_any<P, T, void>::value, T>& EntityHandle<P, C...>::get_component() noexcept
{
	static_assert(impl::is_any<T, C...>::value, "Component not found");
	assert(this->valid_ && "Entity isn't valid");

	return entities_[index_].template get_component<T>();
}

template <typename P, typename... C>
template <typename T>
std::enable_if_t<!std::is_pointer<T>::value, T> const& EntityHandle<P, C...>::get_component() const noexcept
{
	static_assert(impl::is_any<T, C...>::value, "Component not found");
	assert(this->valid_ && "Entity isn't valid");

	return entities_[index_].template get_component<T>();
}

template <typename P, typename... C>
template <typename T>
std::enable_if_t<std::is_pointer<T>::value, std::remove_pointer_t<T>> const* const&
	EntityHandle<P, C...>::get_component() const noexcept
{
	static_assert(impl::is_any<T, C...>::value, "Component not found");
	assert(this->valid_ && "Entity isn't valid");

	return entities_[index_].template get_pointer<T>();
}

template <typename P, typename... C>
template <typename... Ts>
bool EntityHandle<P, C...>::has_components() const noexcept
{
	(void)impl::expand{([]
	{
		static_assert(impl::is_any<Ts, C...>::value, "Component not found");
	}(), 0)...};
	assert(this->valid_ && "Entity isn't valid");

	return entities_[index_].template has_components<Ts...>();
}

template <typename P, typename... C>
template <typename T, typename... Args>
void EntityHandle<P, C...>::add_component(Args&&... args)
{
	static_assert(impl::is_any<T, C...>::value, "Component not found");
	assert(this->valid_ && "Entity isn't valid");

	return entities_[index_].template add_component<T>(std::forward<Args>(args)...);
}

template <typename P, typename... C>
template <typename... Ts>
void EntityHandle<P, C...>::add_components()
{
	(void)impl::expand{([]
	{
		static_assert(impl::is_any<Ts, C...>::value, "Component not found");
	}(), 0)...};
	assert(this->valid_ && "Entity isn't valid");

	entities_[index_].template add_components<Ts...>();
}

template <typename P, typename... C>
template <typename... Ts, typename... Args>
void EntityHandle<P, C...>::add_components(Args&&... args)
{
	(void)impl::expand{([]
	{
		static_assert(impl::is_any<Ts, C...>::value, "Component not found");
	}(), 0)...};
	assert(this->valid_ && "Entity isn't valid");

	entities_[index_].template add_components<Ts...>(std::forward<Args>(args)...);
}

template <typename P, typename... C>
template <typename... Ts>
void EntityHandle<P, C...>::remove_components()
{
	(void)impl::expand{([]
	{
		static_assert(impl::is_any<Ts, C...>::value, "Component not found");
	}(), 0)...};
	assert(this->valid_ && "Entity isn't valid");

	entities_[index_].template remove_components<Ts...>();
}

} // namespace mantra

#endif // Header guard
