// Basic example of the library
// 
// The components are 
//  - a counter (as an int)
//  - an increment tag
//  - a decrement tag
//  
// The systems are
// - the display system. It prints the counter to the standard output
// - the increment system. It takes entities with a counter and an increment tag and increments the counter
//   by 1 until a given threshold. When that value is reached, the increment tag is removed from the entity
//   and a decrement tag is added
// - the decrement system. It does the opposite of the increment system

#include <iostream>

#include <mantra/System.hpp>
#include <mantra/World.hpp>

using Counter = int;
struct IncTag {};
struct DecTag {};

class IncSys : public mantra::System<Counter, IncTag>
{
	public:
	IncSys(int m) : max{m} {}
	
	template <typename WV>
	void update(WV&& wv)
	{
		for (auto& entity : wv.entities())
		{
			if (entity.template get_component<Counter>() < max)
				++entity.template get_component<Counter>();
			else
			{
				entity.template remove_components<IncTag>();
				entity.template add_component<DecTag>();
			}
		}
	}

	private:
	int max;
};

class DecSys : public mantra::System<Counter, DecTag>
{
	public:
	DecSys(int m) : min{m} {}
	
	template <typename WV>
	void update(WV&& wv)
	{
		for (auto& entity : wv.entities())
		{
			if (entity.template get_component<Counter>() > min)
				--entity.template get_component<Counter>();
			else
			{
				entity.template remove_components<DecTag>();
				entity.template add_component<IncTag>();
			}
		}
	}

	private:
	int min;
};

class DisplaySys : public mantra::System<void, Counter>
{
	public:
	template <typename WV>
	void update(WV&& wv)
	{
		for (auto& entity : wv.entities())
			std::cout << entity.template get_component<Counter>() << '\n';
	}
};

int main()
{
	auto comps = mantra::ComponentList<Counter, IncTag, DecTag>{};
	auto sys = mantra::SystemList<IncSys, DecSys, DisplaySys>{};

	auto world = mantra::create_world(comps, sys, mantra::forward_as_tuple(10), mantra::forward_as_tuple(-10),
	                                  mantra::forward_as_tuple());

	auto e1 = world.create_entity<Counter, IncTag>();
	auto e2 = world.create_entity<Counter, DecTag>(mantra::forward_as_tuple(5), mantra::forward_as_tuple());
	auto e3 = world.create_entity<Counter>(mantra::forward_as_tuple(-8));

	e3.add_components<IncTag>();

	while (1)
	{
		world.update();
		int c{std::cin.get()};
		if (c == 'e')
			break;
	}
}
