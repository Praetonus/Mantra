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

template <typename W, typename P, typename... C>
class WorldView final
{
	using WC = impl::WorldCont<typename W::Components>;

	public:
	WorldView(typename WC::EntCont&, typename WC::CompCont&) noexcept;

	WorldView(WorldView const&) = delete;
	WorldView& operator=(WorldView const&) = delete;
	
	WorldView(WorldView&&) = delete;
	WorldView& operator=(WorldView&&) = delete;

	~WorldView() = default;

	template <typename... Ts>
	EntityHandle<W, P, C...> create_entity();

	template <typename... Ts, typename... Args>
	EntityHandle<W, P, C...> create_entity(Args&&...);

	private:
	typename WC::EntCont& entities_;
	typename WC::CompCont& components_;
};

} // namespace mantra

#include "impl/WorldView.hpp"

#endif // Header guard
