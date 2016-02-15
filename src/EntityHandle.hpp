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

template <typename W, typename P, typename... C>
class EntityHandle final
#ifndef NDEBUG
	: public impl::DebugHandle<typename W::Components>
#endif
{
	using WC = impl::WorldCont<typename W::Components>;

	public:
	EntityHandle(typename WC::EntCont&, std::size_t);

	EntityHandle(EntityHandle const&) = default;
	EntityHandle& operator=(EntityHandle const&) = delete;
	
	EntityHandle(EntityHandle&&) = default;
	EntityHandle& operator=(EntityHandle&&) = delete;

	~EntityHandle() = default;

	void destroy();

	template <typename T>
	std::enable_if_t<impl::is_any<P, T, void>{}, T>& get_component() noexcept;

	template <typename T>
	std::enable_if_t<!std::is_pointer<T>{}, T> const& get_component() const noexcept;

	template <typename T>
	std::enable_if_t<std::is_pointer<T>{}, std::remove_pointer_t<T>> const* const&
		get_component() const noexcept;

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

	template <typename... CC>
	friend bool operator==(mantra::EntityHandle<CC...> const&, mantra::EntityHandle<CC...> const&) noexcept;

	private:
	typename WC::EntCont& entities_;
	std::size_t index_;
};

template <typename... C>
bool operator==(mantra::EntityHandle<C...> const& l, mantra::EntityHandle<C...> const& r) noexcept
{
	assert(l.valid_ && r.valid_ && "Entities aren't valid");

	return &l.entities_ == &r.entities_ && l.index_ == r.index_;
}

} // namespace mantra

#include "impl/EntityHandle.hpp"

#endif // Header guard
