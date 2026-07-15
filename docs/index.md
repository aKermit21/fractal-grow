---
title: "exfra Intro"
description: "exfra documentation"
---

**exfra**: **Cosmic Fractal — 2D Animation, Game** 
more descriptive name: ****Ex***ponentially Growing ***Fra***ctal Leaf*
[GitHub repo](https://github.com/aKermit21/fractal-grow/)

# Principle of animation 🎬
Elements of a leaf-shaped fractal are drawn recursively by applying a defined transformation that turns the largest element into progressively smaller ones.
However, from time to time, one of the existing elements undergoes a spontaneous *mutation*. It begins to gradually grow, along with all of its subordinate elements. This creates — or rather modifies — a branch, which ends up usually larger than its predecessor.
After a short time, the mutation process repeats itself within the already mutated branch.

As a result, the fractal structure grows upward exponentially, and the frame rescales to cover more and more area. The structure starts from 24 cm, which is roughly the initial structure size on the screen.
After about 30 minutes of self-perpetuating *Animation*, the fractal reaches the size of the Milky Way Galaxy. More precisely, when the animation is finished, it covers the distance from Earth to the *Edge of the Galaxy*. That time, however, may be shortened by user actions — and here is where the *Game* part comes into play...

# Game 🏆
If, during the mutated growing phase, the stem *catches* light rays of a similar color, that stem becomes *Excited* (super-charged). It will then extend its size even further and faster.
Catching light happens when light rays (illustrated when the light source is in motion) cross perpendicularly to the stem axis. This is indicated by the stem color turning white.
It is up to the player to manipulate the light source while the fractal structure is growing. This is done using the arrow keys.

Since a stem usually has different colors on its two ends, the intermediate color matters. This is represented by a color flag pointing to the middle of the stem.

