diff  src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h
55a56,59
> 
> 	static bool useCGALTriangleCollision_b;
> 	static bool doLogCGALTriangleCollision_b;
> 	
296a301,304
> 	static void useCGALTriangleCollision(bool useCGALTriangleCollision_param);
> 	static bool isUsingCGALTriangleCollision();
> 	static void doLogCGALTriangleCollision(bool doLogCGALTriangleCollision_param);
> 	
301c309
< //#define BULLET_TRIANGLE_COLLISION 1
---
> #define BULLET_TRIANGLE_COLLISION 1
