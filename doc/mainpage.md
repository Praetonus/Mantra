\mainpage Getting started

This guide explains how to use the basic features of the library.

\attention We will not explain what an Entity-Component-System structure is. You can see [here](http://entity-systems.wikidot.com) for information about this.

As an example, we will implement a very simple ECS. Entities will have a counter component holding an integer and a tag component (increment or decrement). Depending on the tag, the counter will be incremented or decremented by different systems. We'll also have a display system which will print the counter.

# Library structure

The classes you'll have to deal with to use the library are

- `World` : The main class. Holds entities and updates systems.
- `System` : A helper class to define systems.
- `EntityHandle` : A reference to an entity. You can retreive, add and remove components through it.
- `WorldView` : A view of the `World` through a system. Controls which components can be accessed.

# Components

Components are simply types. Fundamental types, pointers, structures or classes, but no references. In this example, we will use an integer type alias for the counter and empty `struct`s for the tags.

```{.cpp}
using Counter = int;
struct IncTag {};
struct DecTag {};
```

# Systems

## Primary and secondary components

Primary and secondary components are a fundamental concept in Mantra. A system can have a single primary component and any number of secondary components. In a given system, the primary component is the only mutable component and all secondary components are deeply immutable. A system can have no primary component. In this case, the system can't write to any component.

\warning Deep immutablility means that every object reachable through the component should be considered immutable. This is also true for objects reachable through pointers and references, even if modification is allowed by the rules of the language.

## Defining systems

The most convenient way to define a system is to inherit from `System`. The template parameters of `System` are the components an entity should possess to be visible by the system. The first component specified is the primary component. If it is `void`, there is no primary component.

We'll start by defining the increment system, which will increment the counter if the increment tag is present. When the counter reaches a given threshold, the tag is removed and replaced by a decrement tag.

The logic of the system is implemented in the `update` function. The parameter is a `WorldView`. Iterating over entities yields `EntityHandle`s. It is not possible to access components that are neither primary nor secondary.

```{.cpp}
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
```

Next is the decrement system, which does the opposite of the increment system.

```{.cpp}
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
```

And last is the display system. It doesn't have a primary component.

```{.cpp}
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
```

# World

Now we can create a `World` to use our components and systems together. We'll also show how to create entities. Notice that when multiple components or systems are created at the same time, the parameters for each are passed in tuples.

The `World`'s `update` function runs each system once and returns. Systems are updated in the order of the `World`'s system list.

```{.cpp}
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
```

If you run this example, you'll notice that the counter stays at its minimum for one additional frame. This is caused by the order in which the systems are updated : `IncSys` runs before `DecSys`, so a component can be updated by `IncSys` and then by `DecSys` in the same frame. Be aware of this behavior when writing your own systems.

\note This example is available in `example/basic.cpp`.
