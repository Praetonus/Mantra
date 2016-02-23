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

#ifndef MANTRA_IMPL_WORLDVIEW_HPP
#define MANTRA_IMPL_WORLDVIEW_HPP

#include "../WorldView.hpp"

namespace mantra
{

template <typename W, typename P, typename... C>
WorldView<W, P, C...>::WorldView(typename WC::EntCont& entities, typename WC::CompCont& components,
                                 typename WC::SysCont& systems, typename WC::Caches& caches) noexcept
	: entities_{entities}, components_{components}, systems_{systems}, free_caches_{caches}
{
	impl::validate_components(typename W::Components{}, impl::TypeList<C...>{});
}

template <typename W, typename P, typename... C>
template <typename... Ts>
EntityHandle<W, P, C...> WorldView<W, P, C...>::create_entity()
{
	impl::TypeList<Ts...> comp_types{};
	impl::validate_components(typename W::Components{}, comp_types);

	auto it = std::begin(entities_);
	if (!free_caches_[0].empty())
	{
		auto index = free_caches_[0].back();
		free_caches_[0].pop_back();
		it += static_cast<std::ptrdiff_t>(index);
	}
	else
	{
		it = std::find_if(std::begin(entities_), std::end(entities_),
		                  [](auto const& e){return !e;});
		if (it == std::end(entities_))
		{
			entities_.emplace_back(components_, free_caches_, entities_.size());
			it = std::end(entities_) - 1;
		}
	}
	it->create(comp_types);
	return {entities_, static_cast<std::size_t>(it - std::begin(entities_))};
}

template <typename W, typename P, typename... C>
template <typename... Ts, typename... Args>
EntityHandle<W, P, C...> WorldView<W, P, C...>::create_entity(Args&&... args)
{
	impl::TypeList<Ts...> comp_types{};
	impl::validate_components(typename W::Components{}, comp_types);

	auto it = std::begin(entities_);
	if (!free_caches_[0].empty())
	{
		auto index = free_caches_[0].back();
		free_caches_[0].pop_back();
		it += static_cast<std::ptrdiff_t>(index);
	}
	else
	{
		it = std::find_if(std::begin(entities_), std::end(entities_),
		                  [](auto const& e){return !e;});
		if (it == std::end(entities_))
		{
			entities_.emplace_back(components_, free_caches_, entities_.size());
			it = std::end(entities_) - 1;
		}
	}
	it->create(comp_types, std::forward<Args>(args)...);
	return {entities_, static_cast<std::size_t>(it - std::begin(entities_))};
}

template <typename W, typename P, typename... C>
typename WorldView<W, P, C...>::Entities WorldView<W, P, C...>::entities()
{
	return WorldView<W, P, C...>::Entities{*this};
}

template <typename W, typename P, typename... C>
template <typename T, typename A>
void WorldView<W, P, C...>::message(A&& arg)
{
	impl::get<T>(systems_).receive(std::forward<A>(arg));
}

template <typename W, typename P, typename... C>
void WorldView<W, P, C...>::reserve_entities(std::size_t n)
{
	if (free_caches_[0].size() < n)
		entities_.reserve(entities_.size() + n - free_caches_[0].size());
}

template <typename W, typename P, typename... C>
template <typename T>
void WorldView<W, P, C...>::reserve_components(std::size_t n)
{
	impl::validate_component<T>(typename W::Components{});

	auto& cache = free_caches_[impl::index_of<T, C...>()+1];
	if (cache.size() < n)
	{
		auto& comps = std::get<std::vector<boost::optional<T>>>(components_);
		comps.reserve(comps.size() + n - cache.size());
	}
}

template <typename W, typename P, typename... C>
WorldView<W, P, C...>::Entities::Entities(WorldView<W, P, C...>& view)
	: view_{view}
{}

template <typename W, typename P, typename... C>
typename WorldView<W, P, C...>::EntityIterator WorldView<W, P, C...>::Entities::begin()
{
	return WorldView<W, P, C...>::EntityIterator{view_};
}

template <typename W, typename P, typename... C>
typename WorldView<W, P, C...>::EntityIterator WorldView<W, P, C...>::Entities::end()
{
	return WorldView<W, P, C...>::EntityIterator{};
}

template <typename W, typename P, typename... C>
WorldView<W, P, C...>::EntityIterator::EntityIterator()
	: view_{nullptr}, handle_{}, index_{0}
{}

template <typename W, typename P, typename... C>
WorldView<W, P, C...>::EntityIterator::EntityIterator(WorldView<W, P, C...>& view)
	: view_{&view}, handle_{}, index_{0}
{
	if (view_->entities_.empty())
		view_ = nullptr;
	else if (!view_->entities_[0] || !view_->entities_[0].template has_components<C...>())
		find_next_();
}

template <typename W, typename P, typename... C>
WorldView<W, P, C...>::EntityIterator::EntityIterator(WorldView<W, P, C...>::EntityIterator const& cp)
	: view_{cp.view_}, handle_{}, index_{cp.index_}
{}

template <typename W, typename P, typename... C>
EntityHandle<W, P, C...>& WorldView<W, P, C...>::EntityIterator::operator*()
{
	assert(view_ && "Can't dereference an invalid iterator");

	if (!handle_)
		handle_.emplace(view_->entities_, index_);

	return handle_.get();
}

template <typename W, typename P, typename... C>
EntityHandle<W, P, C...>* WorldView<W, P, C...>::EntityIterator::operator->()
{
	assert(view_ && "Can't dereference an invalid iterator");

	if (!handle_)
		handle_.emplace(view_->entities_, index_);

	return &(handle_.get());
}

template <typename W, typename P, typename... C>
typename WorldView<W, P, C...>::EntityIterator& WorldView<W, P, C...>::EntityIterator::operator++()
{
	assert(view_ && "Can't increment an invalid iterator");

	handle_ = boost::none;
	find_next_();

	return *this;
}

template <typename W, typename P, typename... C>
typename WorldView<W, P, C...>::EntityIterator WorldView<W, P, C...>::EntityIterator::operator++(int)
{
	assert(view_ && "Can't increment an invalid iterator");

	auto cp = *this;
	++*this;

	return cp;
}

template <typename W, typename P, typename... C>
void WorldView<W, P, C...>::EntityIterator::find_next_()
{
	assert(view_ && "(Dev) Can't call this on an invalid iterator");

	auto it = std::find_if(std::begin(view_->entities_) + static_cast<std::ptrdiff_t>(index_) + 1,
	                       std::end(view_->entities_),
	                       [](auto const& e){return e && e.template has_components<C...>();});
	if (it == std::end(view_->entities_))
	{
		view_ = nullptr;
		index_ = 0;
	}
	else
		index_ = static_cast<std::size_t>(it - std::begin(view_->entities_));
}

} // namespace mantra

#endif // Header guard
