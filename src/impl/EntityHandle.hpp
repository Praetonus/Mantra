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

template <typename... C>
EntityHandle<C...>::EntityHandle(std::vector<impl::Entity<C...>>& entities, std::size_t index)
	: entities_{entities}, index_{index}
#ifndef NDEBUG
, valid_{true}
{
	entities_[index_].add_handle(this);
}
#else
{}
#endif // NDEBUG

#ifndef NDEBUG
template <typename... C>
EntityHandle<C...>::EntityHandle(EntityHandle const& cp)
	: entities_{cp.entities_}, index_{cp.index_}, valid_{cp.valid_}
{
	if (valid_)
		entities_[index_].add_handle(this);
}

template <typename... C>
EntityHandle<C...>::EntityHandle(EntityHandle&& mv)
	: entities_{mv.entities_}, index_{mv.index_}, valid_{mv.valid_}
{
	if (valid_)
	{
		entities_[index_].remove_handle(&mv);
		mv.valid_ = false;
		entities_[index_].add_handle(this);
	}
}

template <typename... C>
EntityHandle<C...>::~EntityHandle()
{
	if (valid_)
	{
		entities_[index_].remove_handle(this);
		valid_ = false;
	}
}

template <typename... C>
void EntityHandle<C...>::invalidate_(impl::EntityKey)
{
	valid_ = false;
}
#endif // NDEBUG

template <typename... C>
void EntityHandle<C...>::destroy()
{
	assert(valid_ && "Entity isn't valid");

	entities_[index_].destroy();
}

template <typename... C>
template <typename T>
T& EntityHandle<C...>::get_component() noexcept
{
	static_assert(impl::is_any<T, C...>::value, "Component not found");
	assert(valid_ && "Entity isn't valid");

	return entities_[index_].template get_component<T>();
}

template <typename... C>
template <typename T>
T const& EntityHandle<C...>::get_component() const noexcept
{
	static_assert(impl::is_any<T, C...>::value, "Component not found");
	assert(valid_ && "Entity isn't valid");

	return entities_[index_].template get_component<T>();
}

template <typename... C>
template <typename... Ts>
void EntityHandle<C...>::add_components()
{
	(void)impl::expand{([]
	{
		static_assert(impl::is_any<Ts, C...>::value, "Component not found");
	}(), 0)...};
	assert(valid_ && "Entity isn't valid");

	entities_[index_].template add_components<Ts...>();
}

template <typename... C>
template <typename... Ts, typename Tuple>
void EntityHandle<C...>::add_components(Tuple&& args)
{
	(void)impl::expand{([]
	{
		static_assert(impl::is_any<Ts, C...>::value, "Component not found");
	}(), 0)...};
	assert(valid_ && "Entity isn't valid");

	entities_[index_].template add_components<Ts...>(std::forward<Tuple>(args));
}

template <typename... C>
template <typename... Ts>
void EntityHandle<C...>::remove_components()
{
	(void)impl::expand{([]
	{
		static_assert(impl::is_any<Ts, C...>::value, "Component not found");
	}(), 0)...};
	assert(valid_ && "Entity isn't valid");

	entities_[index_].template remove_components<Ts...>();
}


} // namespace mantra

#endif // Header guard
