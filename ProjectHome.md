**Dynamic Voronoi shatter C++ API for non-convex triangle meshes with BulletPhysics dynamic shatter on objects collision and RenderMan RIB generation**

  * The project implements a Voronoi shatter of non-convex closed triangles meshes using C++ and CGAL
  * The project is used in my Maya Voronoi Shatter C++ Plug-in (see below)
  * The shatter uses the Voronoi brute force algorithm implemented in multithreading
  * The cut faces are rebuilt using the Constrained Delaunay triangulation
  * The triangles mesh can contain multiple holes and cavities with nested objects
  * The algorithm can separate the disjointed surfaces
  * The C++ API can calculate the mesh's volume and the inertia tensor using the volume 3D Triangulation
  * The algorithm has been applied to BulletPhysics to shatter the objects on the collision points
  * I have created: an API for the dynamic objects and scenes description, an OpenGL scene viewer and a RenderMan RIB scene generator with motion blur
  * The RenderMan RIB scenes have been rendered using 3Delight with motion blur
  * BulletPhysics btGImpactCollisionAlgorithm class has been modified to handle the collisions for non-convex triangle meshes
  * The project uses Boost C++ API (Containers, Threads and Serialization) and Intel Threading Building Blocks (TBB)

**The Larmor-Physx project is Free Software released under the GNU/GPL License.**

<a href='http://www.youtube.com/watch?feature=player_embedded&v=2iFlHiCOn-Q' target='_blank'><img src='http://img.youtube.com/vi/2iFlHiCOn-Q/0.jpg' width='853' height=480 /></a>

<a href='http://www.youtube.com/watch?feature=player_embedded&v=-zdFiocwIpQ' target='_blank'><img src='http://img.youtube.com/vi/-zdFiocwIpQ/0.jpg' width='853' height=480 /></a>

<a href='http://www.youtube.com/watch?feature=player_embedded&v=q-NO419qAWc' target='_blank'><img src='http://img.youtube.com/vi/q-NO419qAWc/0.jpg' width='853' height=480 /></a>

<a href='http://www.youtube.com/watch?feature=player_embedded&v=14mil0tEIac' target='_blank'><img src='http://img.youtube.com/vi/14mil0tEIac/0.jpg' width='853' height=480 /></a>

![![](http://www.larmor.com/portfolio/frame_00056_resize.jpg)](http://www.larmor.com/portfolio/frame_00056.jpg) ![![](http://www.larmor.com/portfolio/frame_00110_resize.jpg)](http://www.larmor.com/portfolio/frame_00110.jpg)


---


**LarmorVoronoi Plugin for Maya 2012/2013/2014 (Windows, Mac OS X, Linux)**

LarmorVoronoi is a Voronoi Shatter Plug-in in C++ for Maya 2012/2013/2014 for Windows, Mac OS X and Linux, that uses the Larmor-Physx Voronoi Shatter API.

LarmorVoronoi Maya Plugin documentation: http://www.larmor.com/LarmorVoronoiMayaPlugin/

Plug-in Binary version 1.1.0Beta-Build77 for Maya 2012/2013/2014 64 bit Windows: http://larmor-physx.googlecode.com/svn/trunk/LarmorVoronoi_MayaPlugIn/builds/windows/LarmorVoronoiMayaPlugin-1.1.0Beta-Build77.exe

Plug-in Binary version 1.1.1Beta-Build78 for Maya 2012/2013/2014 Mac OS X: http://larmor-physx.googlecode.com/svn/trunk/LarmorVoronoi_MayaPlugIn/builds/macosx/

Plug-in Binary version 1.1.1Beta-Build79 for Maya 2012/2013/2014 Linux (x86\_64): http://larmor-physx.googlecode.com/svn/trunk/LarmorVoronoi_MayaPlugIn/builds/linux/

CreativeCrash.com plug-in page: http://www.creativecrash.com/maya/plugin/larmorvoronoi-plugin-for-maya-2012-2013-2014

**The LarmorVoronoi Maya Plugin project is Free Software released under the GNU/GPL License.**

<a href='http://www.youtube.com/watch?feature=player_embedded&v=WF31Q60SKTU' target='_blank'><img src='http://img.youtube.com/vi/WF31Q60SKTU/0.jpg' width='800' height=450 /></a>

[![](http://www.larmor.com/portfolio/LarmorVoronoi_Maya2014_screenshot_resize.jpg)](http://www.larmor.com/LarmorVoronoiMayaPlugin/)

[![](http://www.larmor.com/portfolio/Screenshot_mac1_resize640.jpg)](http://www.larmor.com/LarmorVoronoiMayaPlugin/)

[![](http://www.larmor.com/portfolio/LarmorVoronoi_Maya2014_screenshot4_resize.jpg)](http://www.larmor.com/LarmorVoronoiMayaPlugin/)