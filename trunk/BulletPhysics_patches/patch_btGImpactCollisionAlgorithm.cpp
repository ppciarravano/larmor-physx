diff  src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp src/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp
34a35,231
> //Start: Pier Paolo Ciarravano Larmor Patch for Exact triangles collisions
> #include <iostream>
> #include <vector>
> #include <list>
> #include <map>
> #include <set>
> #include <boost/unordered_map.hpp>
> #include <CGAL/intersections.h>
> #include <CGAL/point_generators_3.h>
> #include <CGAL/Bbox_3.h>
> #include <CGAL/box_intersection_d.h>
> #include <CGAL/function_objects.h>
> #include <CGAL/Join_input_iterator.h>
> #include <CGAL/algorithm.h>
> #include <boost/unordered_set.hpp>
> #include <boost/thread.hpp>
> #include <CGAL/Aff_transformation_2.h>
> #include <CGAL/Cartesian.h>
> #include <CGAL/squared_distance_3.h>
> 
> //#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
> //typedef CGAL::Exact_predicates_exact_constructions_kernel   Kernel;
> #include <CGAL/Simple_cartesian.h>
> typedef CGAL::Simple_cartesian<double>   Kernel;
> typedef Kernel::Triangle_3 Triangle;
> typedef Kernel::Point_3 Point;
> typedef Kernel::Plane_3 Plane;
> typedef Kernel::Point_2 KPoint2;
> 
> namespace BulletCGALTriangleCollision
> {
> 
> 	struct TriangleVisitedInfo {
> 
> 		Triangle triangle;
> 		int triangleIndex;
> 		bool visited;
> 
> 		//disjoint mesh index
> 		short int disjointMeshIndex;
> 
> 	};
> 
> 	struct IntOrderedPair {
> 
> 		int first;
> 		int second;
> 
> 	};
> 	//Hash using the Cantor pairing function
> 	struct IntOrderedPairHash {
> 		size_t operator()(const IntOrderedPair& p) const {
> 
> 			int a = p.first;
> 			int b = p.second;
> 			boost::hash<int> hasher;
> 			size_t valHash = hasher((a+b)*(a+b+1)/2 +b);
> 
> 			return valHash;
> 		}
> 	};
> 	struct IntOrderedPairEqual {
> 	  bool operator()(const IntOrderedPair& c1, const IntOrderedPair& c2) const {
> 		//std::cout << "EQ" << std::endl;
> 		return (c1.first == c2.first) && (c1.second == c2.second);
> 	  }
> 	};
> 	typedef boost::unordered_set<IntOrderedPair, IntOrderedPairHash, IntOrderedPairEqual> BoostSetIntOrderedPair;
> 	boost::thread_specific_ptr<BoostSetIntOrderedPair> threadLocalIntOrderedPair;
> 	boost::mutex mutex_a;
> 	boost::mutex mutex_b;
> 
> 	//For Box Collision
> 	typedef CGAL::Box_intersection_d::Box_with_handle_d<double, 3, TriangleVisitedInfo*> BoxInt;
> 
> 	#define DISJOINTMESH_TRIANGLE_SCALE_VALUE 1.01
> 	//Scale triangle, used for improve do_intersect results: scale the triangle on its plane, centred in its barycenter
> 	inline Triangle triangleScale(Triangle t)
> 	{
> 		Plane p = t.supporting_plane();
> 		KPoint2 a = p.to_2d(t.vertex(0));
> 		KPoint2 b = p.to_2d(t.vertex(1));
> 		KPoint2 c = p.to_2d(t.vertex(2));
> 		KPoint2 baryc((a.x() + b.x() +c.x())/3.0, (a.y() + b.y() +c.y())/3.0);
> 		//TODO: usare margine invece di scale
> 		CGAL::Aff_transformation_2<Kernel> translate1(CGAL::TRANSLATION, CGAL::Vector_2<Kernel>(-baryc.x(), -baryc.y()));
> 		CGAL::Aff_transformation_2<Kernel> translate2(CGAL::TRANSLATION, CGAL::Vector_2<Kernel>(baryc.x(), baryc.y()));
> 		CGAL::Aff_transformation_2<Kernel> scale(CGAL::SCALING, DISJOINTMESH_TRIANGLE_SCALE_VALUE);
> 		CGAL::Aff_transformation_2<Kernel> transform = translate2 * (scale * translate1);
> 		KPoint2 an = transform(a);
> 		KPoint2 bn = transform(b);
> 		KPoint2 cn = transform(c);
> 		Point an3 = p.to_3d(an);
> 		Point bn3 = p.to_3d(bn);
> 		Point cn3 = p.to_3d(cn);
> 		Triangle tn(an3, bn3, cn3);
> 		return tn;
> 	}
> 
> 	boost::thread_specific_ptr<int> counterIntersectionCallback;
> 	//was 10000 and 100 : with MAX_COLLISION_COUPLES = 3 non sembra funzionare proprio bene! 
> 	#define MAX_BOXES_INTERSECTIONS_CALLBACK 3000
> 	#define MAX_COLLISION_COUPLES 200
> 
> 	//Thread safe using boost::thread_specific_ptr for global variable
> 	void reportIntersectionCallback(const BoxInt& b1, const BoxInt& b2)
> 	{
> 		mutex_a.lock();
> 	
> 		(*(counterIntersectionCallback.get()))++;
> 		
> 		if ( (*(counterIntersectionCallback.get())) >= MAX_BOXES_INTERSECTIONS_CALLBACK )
> 		{
> 			mutex_a.unlock();
> 			return;
> 		}
> 		
> 		//Elimina collisioni se raggiunto un massimo di coppie
> 		if (threadLocalIntOrderedPair.get()->size() >= MAX_COLLISION_COUPLES)
> 		{
> 			mutex_a.unlock();
> 			return;
> 		}	
> 		
> 		TriangleVisitedInfo *m1 = b1.handle();
> 		TriangleVisitedInfo *m2 = b2.handle();
> 
> 		//Comment this part of the code so we can check all the collision couples
> 		//Elimina coppie dove un triangolo almeno e' gia' stato inserito nelle coppie di collisione
> 		//if ((m1->visited) || (m2->visited))
> 		//{
> 		//	mutex_a.unlock();
> 		//	return;
> 		//}
> 		
> 		//This is always true with the use of CGAL::box_intersection_d
> 		if (m1->disjointMeshIndex != m2->disjointMeshIndex)
> 		{
> 			IntOrderedPair iup;
> 			//This is always true with the use of CGAL::box_intersection_d
> 			if (m1->disjointMeshIndex == 0)
> 			{
> 				iup.first = m1->triangleIndex;
> 				iup.second = m2->triangleIndex;
> 			}
> 			else
> 			{
> 				iup.second = m1->triangleIndex;
> 				iup.first = m2->triangleIndex;
> 			}
> 
> 			//DONE with (m1->triangle).supporting_plane() != (m2->triangle).[opposite()].supporting_plane() 
> 			//TODO: se triangoli stanno sullo stesso piano allora non considerarli in collisione
> 			// Utilizzare altra funzione per collisione di CGAL, usata gia in TestCGAL_1
> 			//CGAL::Object intersectionResult = CGAL::intersection(*(m1->triangle), *(m2->triangle));
> 			//   if (const CGAL::Triangle_3<Kernel> *itriangle = CGAL::object_cast<CGAL::Triangle_3<Kernel> >(&intersectionResult))
> 			//--------------
> 			
> 			BoostSetIntOrderedPair::iterator foundIntOrderedPair = threadLocalIntOrderedPair.get()->find(iup);
> 			if (foundIntOrderedPair == threadLocalIntOrderedPair.get()->end())
> 			{
> 				//std::cout << m1->disjointMeshIndex << " - " << m2->disjointMeshIndex << std::endl;
> 				if ( ! m1->triangle.is_degenerate() && ! m2->triangle.is_degenerate() )
> 				{
> 					//TODO: Valutare se utilizzare condizione sui piani e se per motivi di performance utilizzarla qui
> 					// o dopo il test di intersezione triangoli
> 					//if ( ( (m1->triangle).supporting_plane() != (m2->triangle).supporting_plane() ) &&
> 					//	( (m1->triangle).supporting_plane().opposite() != (m2->triangle).supporting_plane() ) )
> 					//{
> 						if (CGAL::do_intersect( (m1->triangle), (m2->triangle) ) )
> 						//if (CGAL::do_intersect( triangleScale((m1->triangle)), triangleScale((m2->triangle)) ) )
> 						{
> 							//std::cout << " INTERSECT!! " << std::endl;
> 							threadLocalIntOrderedPair.get()->insert(iup);
> 							m1->visited = true;
> 							m2->visited = true;
> 						}
> 					//}
> 					//else
> 					//{
> 					//	std::cout << " SAME PLANE!!" << std::endl;
> 					//}
> 				}
> 			}
> 		}
> 
> 		mutex_a.unlock();
> 	}
> 
> } //Close namespace BulletCGALTriangleCollision
> 
> using namespace BulletCGALTriangleCollision;
> 
> //End: Pier Paolo Ciarravano Larmor Patch for Exact triangles collisions
> 
> 
> 
260c457
< 
---
> 		
479c676
< 
---
> 	
490d686
< 
492d687
< 
494c689
< 
---
> 	
504c699,842
< 	gimpact_vs_gimpact_find_pairs(orgtrans0,orgtrans1,shape0,shape1,pairset);
---
> 	
> 	//Start: Pier Paolo Ciarravano Larmor Patch for Exact triangles collisions
> 	if (useCGALTriangleCollision_b)
> 	{
> 		//std::cout << "btGImpactCollisionAlgorithm is using CGAL Triangle collision Algorithm" << std::endl;
> 		
> 		if(shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART &&
> 			shape1->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART)
> 		{
> 			mutex_b.lock();
> 		
> 			//btVector3 tv0 = orgtrans0.getOrigin();
> 			//btQuaternion tq0 = orgtrans0.getRotation();
> 			//std::cout << "Transform 0: " << tv0.getX() << ", " << tv0.getY() << ", " << tv0.getZ() << "  Rot:" << tq0.getX() << ", " << tq0.getY() << ", " << tq0.getZ() << ", " << tq0.getW() << std::endl;
> 			
> 			//btVector3 tv1 = orgtrans1.getOrigin();
> 			//btQuaternion tq1 = orgtrans1.getRotation();
> 			//std::cout << "Transform 1: " << tv1.getX() << ", " << tv1.getY() << ", " << tv1.getZ() << "  Rot:" << tq1.getX() << ", " << tq1.getY() << ", " << tq1.getZ() << ", " << tq1.getW() << std::endl;
> 		
> 		
> 			//check AABB boxes collisions
> 			btVector3 minAABB0;
> 			btVector3 maxAABB0;
> 			shape0->getAabb(orgtrans0, minAABB0, maxAABB0);
> 			CGAL::Bbox_3 tBBoxShape0(minAABB0.getX(), minAABB0.getY(), minAABB0.getZ(),
> 						maxAABB0.getX(), maxAABB0.getY(), maxAABB0.getZ());
> 			btVector3 minAABB1;
> 			btVector3 maxAABB1;
> 			shape1->getAabb(orgtrans1, minAABB1, maxAABB1);
> 			CGAL::Bbox_3 tBBoxShape1(minAABB1.getX(), minAABB1.getY(), minAABB1.getZ(),
> 						maxAABB1.getX(), maxAABB1.getY(), maxAABB1.getZ());
> 			if (! CGAL::do_intersect( tBBoxShape0, tBBoxShape1 ) )
> 			{
> 				mutex_b.unlock();
> 				return;
> 			}
> 			//std::cout << "Box 0 : " << minAABB0.getX() << ", " << minAABB0.getY() << ", " << minAABB0.getZ() << ", " << maxAABB0.getX() << ", " << maxAABB0.getY() << ", " << maxAABB0.getZ() << std::endl;
> 			//std::cout << "Box 1 : " << minAABB1.getX() << ", " << minAABB1.getY() << ", " << minAABB1.getZ() << ", " << maxAABB1.getX() << ", " << maxAABB1.getY() << ", " << maxAABB1.getZ() << std::endl;
> 		
> 			const btGImpactMeshShapePart * shapepart0 = static_cast<const btGImpactMeshShapePart * >(shape0);
> 			const btGImpactMeshShapePart * shapepart1 = static_cast<const btGImpactMeshShapePart * >(shape1);
> 			
> 			//Aggiungo triangoli
> 			
> 			//Add triangles for shapepart0
> 			std::vector<BoxInt> boxes0;
> 			btTriangleShapeEx tri0;
> 			shapepart0->lockChildShapes();
> 			int i0 = shapepart0->getNumChildShapes();
> 			//printf("getNumChildShapes 0: %d\n", i0);
> 			while(i0--)
> 			{
> 				shapepart0->getBulletTriangle(i0, tri0);
> 				tri0.applyTransform(orgtrans0);
> 				
> 				btVector3 p0 = tri0.m_vertices1[0];
> 				btVector3 p1 = tri0.m_vertices1[1];
> 				btVector3 p2 = tri0.m_vertices1[2];
> 				
> 				Triangle tCgal(Point(p0.getX(), p0.getY(), p0.getZ()),
> 								Point(p1.getX(), p1.getY(), p1.getZ()),
> 								Point(p2.getX(), p2.getY(), p2.getZ()) );
> 				
> 				TriangleVisitedInfo *tvi = new TriangleVisitedInfo;
> 				tvi->triangle = tCgal;
> 				tvi->visited = false;
> 				tvi->disjointMeshIndex = 0; //it must be 1 for the shapepart1 triangles
> 				tvi->triangleIndex = i0;
> 				/*
> 				CGAL::Bbox_3 tBBox = tvi->triangle->bbox();
> 				double delta = 0.1;
> 				CGAL::Bbox_3 tBBoxGrow(tBBox.xmin() - delta, tBBox.ymin() - delta, tBBox.zmin() - delta,
> 						tBBox.xmax() + delta, tBBox.ymax() + delta, tBBox.zmax() + delta);
> 				boxes.push_back( BoxInt( tBBoxGrow, tvi ));
> 				*/
> 				boxes0.push_back( BoxInt( tCgal.bbox(), tvi ));
> 				
> 			}
> 			shapepart0->unlockChildShapes();
> 			
> 			//Add triangles for shapepart1
> 			std::vector<BoxInt> boxes1;
> 			btTriangleShapeEx tri1;
> 			shapepart1->lockChildShapes();
> 			int i1 = shapepart1->getNumChildShapes();
> 			while(i1--)
> 			{
> 				shapepart1->getBulletTriangle(i1, tri1);
> 				tri1.applyTransform(orgtrans1);
> 				
> 				btVector3 p0 = tri1.m_vertices1[0];
> 				btVector3 p1 = tri1.m_vertices1[1];
> 				btVector3 p2 = tri1.m_vertices1[2];
> 				
> 				Triangle tCgal(Point(p0.getX(), p0.getY(), p0.getZ()),
> 								Point(p1.getX(), p1.getY(), p1.getZ()),
> 								Point(p2.getX(), p2.getY(), p2.getZ()) );
> 				
> 				TriangleVisitedInfo *tvi = new TriangleVisitedInfo;
> 				tvi->triangle = tCgal;
> 				tvi->visited = false;
> 				tvi->disjointMeshIndex = 1; //it must be 1 for the shapepart1 triangles
> 				tvi->triangleIndex = i1;
> 				
> 				boxes1.push_back( BoxInt( tCgal.bbox(), tvi ));
> 			
> 			}
> 			shapepart1->unlockChildShapes();
> 			
> 			//Init BoostSetIntOrderedPair in the thread
> 			threadLocalIntOrderedPair.reset(new BoostSetIntOrderedPair);
> 			threadLocalIntOrderedPair.get()->clear(); //init redundant
> 
> 			counterIntersectionCallback.reset(new int(0));
> 			*(counterIntersectionCallback.get()) = 0; //init Redundant
> 			
> 			//Do intersection
> 			//CGAL::box_self_intersection_d( boxes.begin(), boxes.end(), reportIntersectionCallback);
> 			CGAL::box_intersection_d( boxes0.begin(), boxes0.end(), boxes1.begin(), boxes1.end(), reportIntersectionCallback);
> 
> 			//LOG FOR INTERSECTIONS 
> 			if (doLogCGALTriangleCollision_b)
> 			{
> 				if (threadLocalIntOrderedPair.get()->size() != 0)
> 					std::cout << "threadLocalIntOrderedPair " << shape0 <<":" << shapepart0->getNumChildShapes() << ", " << shape1 << ":" << shapepart1->getNumChildShapes() << " size: " << threadLocalIntOrderedPair.get()->size() << " intersCallbacks: " << (*(counterIntersectionCallback.get())) << std::endl;
> 			}
> 			
> 			delete counterIntersectionCallback.get();
> 			counterIntersectionCallback.release();
> 			
> 			//cycle on boxes and delete tvi
> 			std::vector<BoxInt>::iterator vectorBoxesIter;
> 			for(vectorBoxesIter = boxes0.begin(); vectorBoxesIter != boxes0.end(); ++vectorBoxesIter)
> 			{
> 				BoxInt boxInt = *vectorBoxesIter;
> 				TriangleVisitedInfo *tviHandled = boxInt.handle();
> 				delete tviHandled;
> 			}
> 			for(vectorBoxesIter = boxes1.begin(); vectorBoxesIter != boxes1.end(); ++vectorBoxesIter)
> 			{
> 				BoxInt boxInt = *vectorBoxesIter;
> 				TriangleVisitedInfo *tviHandled = boxInt.handle();
> 				delete tviHandled;
> 			}
506c844,907
< 	if(pairset.size()== 0) return;
---
> 			//Cycle on collision triangles and run convex_vs_convex_collision
> 			BoostSetIntOrderedPair::iterator setIntOrderedPairIter;
> 			for(setIntOrderedPairIter = threadLocalIntOrderedPair.get()->begin(); setIntOrderedPairIter != threadLocalIntOrderedPair.get()->end(); ++setIntOrderedPairIter)
> 			{
> 				int first = (*setIntOrderedPairIter).first;
> 				int second = (*setIntOrderedPairIter).second;
> 				//std::cout << "ADJ: (" << first << ", " << second << ") " << std::endl;
> 				
> 				
> 				//Collide collide_gjk_triangles method				
> 				btTriangleShapeEx t0;
> 				btTriangleShapeEx t1;
> 				shapepart0->lockChildShapes();
> 				shapepart1->lockChildShapes();
> 				shapepart0->getBulletTriangle(first, t0);
> 				shapepart1->getBulletTriangle(second, t1);
> 				
> 				convex_vs_convex_collision(body0Wrap,body1Wrap,&t0,&t1);
> 						
> 				shapepart0->unlockChildShapes();
> 				shapepart1->unlockChildShapes();
> 				
> 				
> 				/*
> 				//Collide collide_sat_triangles method
> 				GIM_TRIANGLE_CONTACT contact_data;
> 				btPrimitiveTriangle ptri0;
> 				btPrimitiveTriangle ptri1;
> 				shapepart0->lockChildShapes();
> 				shapepart1->lockChildShapes();
> 				shapepart0->getPrimitiveTriangle(first,ptri0);
> 				shapepart1->getPrimitiveTriangle(second,ptri1);
> 				ptri0.applyTransform(orgtrans0);
> 				ptri1.applyTransform(orgtrans1);
> 				ptri0.buildTriPlane();
> 				ptri1.buildTriPlane();
> 				if(ptri0.find_triangle_collision_clip_method(ptri1,contact_data))
> 				{
> 					int j = contact_data.m_point_count;
> 					while(j--)
> 					{
> 
> 						addContactPoint(body0Wrap, body1Wrap,
> 									contact_data.m_points[j],
> 									contact_data.m_separating_normal,
> 									-contact_data.m_penetration_depth);
> 					}
> 				}				
> 				shapepart0->unlockChildShapes();
> 				shapepart1->unlockChildShapes();
> 				*/
> 				
> 			}
> 				
> 			//Release the pointer to BoostSetIntOrderedPair
> 			delete threadLocalIntOrderedPair.get();
> 			threadLocalIntOrderedPair.release();
> 					
> 			mutex_b.unlock();
> 			return;
> 		
> 		}
> 		
> 		gimpact_vs_gimpact_find_pairs(orgtrans0,orgtrans1,shape0,shape1,pairset);
508,509c909,912
< 	if(shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART &&
< 		shape1->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART)
---
> 		if(pairset.size()== 0) return;		
> 		
> 	}
> 	else
511,520c914,934
< 		const btGImpactMeshShapePart * shapepart0 = static_cast<const btGImpactMeshShapePart * >(shape0);
< 		const btGImpactMeshShapePart * shapepart1 = static_cast<const btGImpactMeshShapePart * >(shape1);
< 		//specialized function
< 		#ifdef BULLET_TRIANGLE_COLLISION
< 		collide_gjk_triangles(body0Wrap,body1Wrap,shapepart0,shapepart1,&pairset[0].m_index1,pairset.size());
< 		#else
< 		collide_sat_triangles(body0Wrap,body1Wrap,shapepart0,shapepart1,&pairset[0].m_index1,pairset.size());
< 		#endif
< 
< 		return;
---
> 		//std::cout << "btGImpactCollisionAlgorithm is using BULLET_TRIANGLE_COLLISION" << std::endl;
> 		
> 		gimpact_vs_gimpact_find_pairs(orgtrans0,orgtrans1,shape0,shape1,pairset);
> 
> 		if(pairset.size()== 0) return;
> 
> 		if(shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART &&
> 			shape1->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART)
> 		{
> 			const btGImpactMeshShapePart * shapepart0 = static_cast<const btGImpactMeshShapePart * >(shape0);
> 			const btGImpactMeshShapePart * shapepart1 = static_cast<const btGImpactMeshShapePart * >(shape1);
> 			//specialized function
> 			#ifdef BULLET_TRIANGLE_COLLISION
> 			collide_gjk_triangles(body0Wrap,body1Wrap,shapepart0,shapepart1,&pairset[0].m_index1,pairset.size());
> 			#else
> 			collide_sat_triangles(body0Wrap,body1Wrap,shapepart0,shapepart1,&pairset[0].m_index1,pairset.size());
> 			#endif
> 
> 			return;
> 		}		
> 		
521a936
> 	//End: Pier Paolo Ciarravano Larmor Patch for Exact triangles collisions
522a938
> 	
822c1238
<     m_resultOut = resultOut;
---
> 	m_resultOut = resultOut;
880a1297,1322
> 
> 
> bool btGImpactCollisionAlgorithm::useCGALTriangleCollision_b = true;
> void btGImpactCollisionAlgorithm::useCGALTriangleCollision(bool useCGALTriangleCollision_param)
> {
> 	btGImpactCollisionAlgorithm::useCGALTriangleCollision_b = useCGALTriangleCollision_param;
> 	if (useCGALTriangleCollision_b)
> 	{
> 		std::cout << "btGImpactCollisionAlgorithm is using CGAL Triangle collision Algorithm" << std::endl;
> 	}
> 	else
> 	{
> 		std::cout << "btGImpactCollisionAlgorithm is using BULLET_TRIANGLE_COLLISION" << std::endl;
> 	}
> }
> bool btGImpactCollisionAlgorithm::isUsingCGALTriangleCollision()
> {
> 	return btGImpactCollisionAlgorithm::useCGALTriangleCollision_b;
> }	
> bool btGImpactCollisionAlgorithm::doLogCGALTriangleCollision_b = false;
> void btGImpactCollisionAlgorithm::doLogCGALTriangleCollision(bool doLogCGALTriangleCollision_param)
> {
> 	btGImpactCollisionAlgorithm::doLogCGALTriangleCollision_b = doLogCGALTriangleCollision_param;
> }
> 
> 
