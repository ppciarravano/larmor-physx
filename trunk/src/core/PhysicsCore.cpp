/*
 * Project name: Larmor-Physx
 * Released: 14 June 2013
 * Author: Pier Paolo Ciarravano
 * http://www.larmor.com
 *
 * License: This project is released under the Qt Public License (QPL - OSI-Approved Open Source license).
 * http://opensource.org/licenses/QPL-1.0
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 */

#include "PhysicsCore.h"

#define BT_USE_DOUBLE_PRECISION 1

//for the Bullet Physics friction and restitution callback
extern ContactAddedCallback		gContactAddedCallback;

//#define SHOW_NUM_DEEP_PENETRATIONS
#ifdef SHOW_NUM_DEEP_PENETRATIONS
extern int gNumDeepPenetrationChecks;
extern int gNumSplitImpulseRecoveries;
extern int gNumGjkChecks;
#endif

//#defind LOG_COLLISION_CALLBACK

namespace LarmorPhysx
{

			//Used to store the contact point for each preTickCallbackLog call
			// access using boost::mutex lock
			CollisionPointVector collisionPointVector;
			boost::mutex mutexCollisionPointVector;

			PhysicsCore::PhysicsCore()
			{
				std::cout << "Starting PhysicsCore..." << std::endl;

				initPhysics();
				initWorld();

				//Animation loop
				for (unsigned int frameCounter = 0;
						frameCounter < LarmorPhysx::ConfigManager::total_anim_steps;
						++frameCounter)
				{

					//Step
					stepWorld();

					//Output on new Frame with idFrame = this.idFrame
					renderWorld();

					//Update the world: voronoi shatter
					updateWorld();

					//Apply External forces
					//applyExternalForces();

				}


				//clear all
				destroyWorld();
			}


			PhysicsCore::~PhysicsCore()
			{

			}


			void PhysicsCore::initPhysics()
			{
				std::cout << "PhysicsCore::initPhysics" << std::endl;

				//Setting not_use_cgal_collision == false then is using CGAL Triangles collision in Bullet Physics
				if (LarmorPhysx::ConfigManager::not_use_cgal_collision)
				{
					btGImpactCollisionAlgorithm::useCGALTriangleCollision(false);
				}
				std::cout << "isUsingCGALTriangleCollision: " << btGImpactCollisionAlgorithm::isUsingCGALTriangleCollision() << std::endl;

				//Not do the custom CGAL Collision log
				btGImpactCollisionAlgorithm::doLogCGALTriangleCollision(false);


				//Init the Bullet Physics friction and restitution callback
				//http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Callbacks_and_Triggers
				gContactAddedCallback = LarmorPhysx::customMaterialCombinerCallback;

				//Init Bullet Physics
				btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
				btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
				//btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

				int  maxProxies = 3000000;
				btVector3 worldAabbMin(-10000,-10000,-10000);
				btVector3 worldAabbMax( 10000, 10000, 10000);
				btBroadphaseInterface* broadphase = new bt32BitAxisSweep3(worldAabbMin, worldAabbMax, maxProxies);
				btConstraintSolver* constraintSolver = new btSequentialImpulseConstraintSolver();

				//init btDynamicsWorld class member
				m_dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,constraintSolver,collisionConfiguration);
				//Register GIMPACT algorithm
				btCollisionDispatcher * dispatcherFromDynamicsWorld = static_cast<btCollisionDispatcher *>(m_dynamicsWorld ->getDispatcher());
				btGImpactCollisionAlgorithm::registerAlgorithm(dispatcherFromDynamicsWorld);

				m_dynamicsWorld->setGravity(btVector3(0, LarmorPhysx::ConfigManager::gravity_force, 0));

				//Split impulse
				//http://bulletphysics.org/mediawiki-1.5.8/index.php/BtContactSolverInfo
				//m_dynamicsWorld->getSolverInfo().m_splitImpulse = true; //it works better without split impulse
				//optionally set the m_splitImpulsePenetrationThreshold (only used when m_splitImpulse  is enabled)
				// only enable split impulse position correction when the penetration is deeper than
				// this m_splitImpulsePenetrationThreshold, otherwise use the regular velocity/position constraint coupling.
				//m_dynamicsWorld->getSolverInfo().m_splitImpulsePenetrationThreshold = -0.01;

				//Callback for  Sub Step
				m_dynamicsWorld->setInternalTickCallback(preTickCallbackLog, this, true);

			}


			void PhysicsCore::initWorld()
			{
				std::cout << "PhysicsCore::initWorld" << std::endl;

				//Init static ground floor, with origin (0, 0, 0), if is_ground_present == 1
				if (LarmorPhysx::ConfigManager::is_ground_present)
				{
					std::cout << "PhysicsCore creating static groud floor..." << std::endl;

					btCollisionShape* staticboxShape = new btBoxShape(btVector3(1000, 1, 1000));

					btTransform	groundTransform;
					groundTransform.setIdentity();
					groundTransform.setOrigin(btVector3(0.0, -0.5, 0.0));

					btCompoundShape* staticScenario = new btCompoundShape();
					staticScenario->addChildShape(groundTransform, staticboxShape);

					btDefaultMotionState* myMotionStateStatic = new btDefaultMotionState(groundTransform);
					btRigidBody::btRigidBodyConstructionInfo cInfo(0.0, myMotionStateStatic, staticScenario, btVector3(0.0f, 0.0f, 0.0f));
					btRigidBody* staticBody = new btRigidBody(cInfo);
					staticBody->setContactProcessingThreshold(0.0);
					//staticBody->setActivationState(DISABLE_DEACTIVATION);

					//Set NULL pointer for the static plane into the user pointer of body
					staticBody->setUserPointer(NULL);

					//Add to the world
					m_dynamicsWorld->addRigidBody(staticBody);

					//Static object
					staticBody->setCollisionFlags(staticBody->getCollisionFlags()|btCollisionObject::CF_STATIC_OBJECT);

					//enable custom material callback with ContactAddedCallback
					staticBody->setCollisionFlags(staticBody->getCollisionFlags()|btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

				}


				std::cout << "PhysicsCore creating objects from input..." << std::endl;

				//Load start frame
				idFrame = LarmorPhysx::ConfigManager::start_load_frame;
				loadStartFrame(LarmorPhysx::ConfigManager::start_load_frame);

				timePosition = stepFrame.timePosition;

				//init dynamicObjectsInWorld and statusInWorld
				dynamicObjectsInWorld.release();
				DynamicObjectVector::iterator dynamicObjectVectorIter;
				for(dynamicObjectVectorIter = stepFrame.dynamicObjects.begin();
						dynamicObjectVectorIter != stepFrame.dynamicObjects.end();
						++dynamicObjectVectorIter)
				{
					dynamicObjectVectorIter->statusInWorld = 0;
					//new and clone the object *dynamicObjectsInWorld
					DynamicObject* dynamicObject = new DynamicObject(*dynamicObjectVectorIter);

					//Load here the dynamicObject->staticObject if use_memory_for_meshes
					if (LarmorPhysx::ConfigManager::use_memory_for_meshes)
					{
						//load staticObject from file
						dynamicObject->staticObject = LarmorPhysx::loadStaticObject( dynamicObject->idObj );
					}

					//add in dynamicObjectsInWorld
					dynamicObjectsInWorld.push_back(dynamicObject);
				}

				//Build frame objects into the world
				idGroupCounter = 0;
				addDynamicObjectsToWorld();

			}


			void PhysicsCore::stepWorld()
			{
				idFrame++;
				std::cout << "PhysicsCore::stepWorld compute Frame N.: " << idFrame << std::endl;

				mutexCollisionPointVector.lock();
				collisionPointVector.clear();
				mutexCollisionPointVector.unlock();

				//1.0L (long double)
				m_dynamicsWorld->stepSimulation(1.0f/LarmorPhysx::ConfigManager::steps_per_second,
						10000,
						1.0f/(LarmorPhysx::ConfigManager::steps_per_second * LarmorPhysx::ConfigManager::internal_sub_steps));

				timePosition += 1.0f/LarmorPhysx::ConfigManager::steps_per_second;
			}


			void PhysicsCore::renderWorld()
			{
				std::cout << "PhysicsCore::renderWorld render Frame N.: " << idFrame << "  timePosition: " << timePosition << std::endl;

				//Run collision callback
				preTickCallbackLog(m_dynamicsWorld, 0.0);

				//Populate the max collisions
				maxMapCollisionPointPair.clear();
				maxMapCollisionPoint.clear();
				mutexCollisionPointVector.lock();
				std::cout << "Total Collisions: " << collisionPointVector.size() << std::endl;
				LarmorPhysx::getMaxCollisionPoint(collisionPointVector, maxMapCollisionPointPair, maxMapCollisionPoint);
				CollisionPointVector maxCollisionPointForObject = LarmorPhysx::getCollisionPointVectorFromMap(maxMapCollisionPointPair);
				stepFrame.collisionPoints = maxCollisionPointForObject;
				mutexCollisionPointVector.unlock();

				int numObjects = m_dynamicsWorld->getNumCollisionObjects();
				//std::cout << "numObjects: " << numObjects << std::endl;
				//Cicle on all the objects in the world
				for (int i=0; i<numObjects; ++i)
				{
					btCollisionObject* collisionObj = m_dynamicsWorld->getCollisionObjectArray()[i];
					btRigidBody* body = btRigidBody::upcast(collisionObj);

					if (body && body->getMotionState() && body->getUserPointer())
					{
						DynamicObject* dynamicObject = (DynamicObject*)(body->getUserPointer());

						//std::cout << "renderWorld DynamicObject idObj: " << dynamicObject->idObj << " ["<< dynamicObject << "] " <<
						//		" (" << (i+1) << "/" << numObjects << ")" << std::endl;
						btDefaultMotionState* motionState = (btDefaultMotionState*)body->getMotionState();

						//Get body position and velocity
						btTransform objTransform = motionState->m_graphicsWorldTrans;
						btVector3 objOrigin = objTransform.getOrigin();
						btQuaternion objRotation = objTransform.getRotation();
						btVector3 objRotationAxis = objRotation.getAxis();
						btScalar rotationAngle = objRotation.getAngle(); //Radiant angle
						btVector3 linearVelocity = body->getLinearVelocity();
						btVector3 angularVelocity = body->getAngularVelocity();

						//std::cout << " rotationAngle ---> " << (rotationAngle * (180.0 / 3.14159)) << std::endl;
						//std::cout << " position ---> " << objOrigin.getX() << ", "
						//			<< objOrigin.getY() << ", "
						//			<< objOrigin.getZ() << std::endl;

						//Set DynamicObject attribute
						dynamicObject->position = getLFromBtVector3(objOrigin);
						dynamicObject->rotationAngle = rotationAngle;
						dynamicObject->rotationAxis = getLFromBtVector3(objRotationAxis);
						dynamicObject->linearVelocity = getLFromBtVector3(linearVelocity);
						dynamicObject->angularVelocity = getLFromBtVector3(angularVelocity);

					}

				}

				//Set frame id
				stepFrame.idFrame = idFrame;
				//Set frame timePosition
				stepFrame.timePosition = timePosition;
				//Set frame dynamicObjects
				copyDynamicObjectsInWorldIntoFrame();
				//Save Frame
				LarmorPhysx::saveFrame(stepFrame);

			}


			//Core function for Voronoi Shatter
			void PhysicsCore::updateWorld()
			{
				std::cout << "PhysicsCore::updateWorld render Frame N.: " << idFrame << std::endl;

				//Does Voronoi shatter for each object
				if (LarmorPhysx::ConfigManager::voronoi_shatter)
				{
					bool newObjectIntoDynamicObjectsInWorld = false;
					int numObjects = m_dynamicsWorld->getNumCollisionObjects();
					//std::cout << "numObjects: " << numObjects << std::endl;

					//Use a vector of btCollisionObject* because each time removeRigidBody is called the m_dynamicsWorld->getCollisionObjectArray()[] changes
					std::vector<btCollisionObject*> collisionObjectArray;
					for (int i=0; i<numObjects; ++i)
					{
						btCollisionObject* collisionObj = m_dynamicsWorld->getCollisionObjectArray()[i];
						collisionObjectArray.push_back(collisionObj);
					}

					//Cicle on all the objects in the world
					for (int i=0; i<numObjects; ++i)
					{
						btCollisionObject* collisionObj = collisionObjectArray.at(i);
						btRigidBody* body = btRigidBody::upcast(collisionObj);

						if (body && body->getMotionState() && body->getUserPointer())
						{
							DynamicObject* dynamicObject = (DynamicObject*)(body->getUserPointer());

							//std::cout << "updateWorld DynamicObject idObj: " << dynamicObject->idObj << " ["<< dynamicObject << "] " <<
							//		" (" << (i+1) << "/" << numObjects << ")" << (m_dynamicsWorld->getNumCollisionObjects()) << std::endl;
							btDefaultMotionState* motionState = (btDefaultMotionState*)body->getMotionState();

							//This should be always true
							if (dynamicObject->idObj != 0)
							{
								MapObjectIdCollisionPoint::iterator foundCollision = maxMapCollisionPoint.find(dynamicObject->idObj);
								if (foundCollision != maxMapCollisionPoint.end())
								{
									CollisionPoint cpMax = foundCollision->second;
									if (cpMax.forceModule >= dynamicObject->staticObject.hardnessCoefficient)
									{
										std::cout << "Doing Voronoi Shatter: objId: " << dynamicObject->idObj << " collision impulse: " << cpMax.forceModule <<
												" point: ("<< cpMax.contactPoint.x << ", " << cpMax.contactPoint.y << ", " << cpMax.contactPoint.z << ")" << std::endl;

										//Load dynamicObject->staticObject if not use_memory_for_meshes
										if (! LarmorPhysx::ConfigManager::use_memory_for_meshes)
										{
											//Load from file
											dynamicObject->staticObject = LarmorPhysx::loadStaticObject( dynamicObject->idObj );
										}
										//else dynamicObject->staticObject should be already in memory

										//Build Complete shattered pieces of mesh
										int numShatterPieces = dynamicObject->staticObject.volume / dynamicObject->staticObject.breakCoefficient;
										if (numShatterPieces >= 2)
										{
											std::cout << " numShatterPieces: " << numShatterPieces << std::endl;
											//Point parentOriginObj = staticObject.meshData.second;
											//Call shatter function
											std::list<MeshData> shatterMeshesNoCentered = voronoiShatter_uniformDistributionPoints(dynamicObject->staticObject.meshData.first, numShatterPieces, false);
											std::list<TranslatedMeshData> shatterMeshes = centerMeshesInBarycenter(shatterMeshesNoCentered);

											//Add voronoi pieces to the world
											unsigned int previousLastCreatedIdObj = stepFrame.lastCreatedIdObj;
											std::list<TranslatedMeshData>::iterator meshDataInter;
											for(meshDataInter = shatterMeshes.begin(); meshDataInter != shatterMeshes.end(); ++meshDataInter)
											{
												TranslatedMeshData meshDataPartTranslated = *meshDataInter;
												stepFrame.lastCreatedIdObj++;

												std::cout << " Creating Object idObj:" << stepFrame.lastCreatedIdObj << "  N." << (stepFrame.lastCreatedIdObj - previousLastCreatedIdObj) << "/" << numShatterPieces << std::endl;

												//Create StaticObject
												StaticObject so;
												so.idObj = stepFrame.lastCreatedIdObj;
												so.idParentObj = dynamicObject->idObj;
												so.meshData = meshDataPartTranslated;
												so.density = dynamicObject->staticObject.density;
												so.breakCoefficient = dynamicObject->staticObject.breakCoefficient;
												so.hardnessCoefficient = dynamicObject->staticObject.hardnessCoefficient;
												so.calculateVolumeMassInertia();
												so.simplifyMesh = TrianglesList();
												if (ConfigManager::use_simplified_meshes_for_bullet)
												{
													so.simplifyMesh = meshSimplification(so.meshData.first.first, 500);
												}

												//Prevent static object is mass and inertia is 0
												if (so.volume < 0.1)
												{
													so.volume = 0.1;
													so.mass = so.volume * so.density;
												}
												if (so.inertia.x < 0.0001)
													so.inertia.x = 0.001;
												if (so.inertia.y < 0.0001)
													so.inertia.y = 0.001;
												if (so.inertia.z < 0.0001)
													so.inertia.z = 0.001;

												std::cout << "---> Object Mass:" << so.mass
														<< " Inertia: (" << so.inertia.x << ", " << so.inertia.y << ", " << so.inertia.z << ")" << std::endl;

												//Save staticObject
												LarmorPhysx::saveStaticObject(so);

												//Create DynamicObject
												DynamicObject dobj;
												dobj.idObj = stepFrame.lastCreatedIdObj;
												idGroupCounter++;
												dobj.idGroup = idGroupCounter; //Each pieces has now a different groupId so m_combinedRestitution = 1.0
												//dobj.idGroup = dynamicObject->idGroup;
												//Calcolate position
												Point originObj = so.meshData.second;
												double cm_x = CGAL::to_double(originObj.x());
												double cm_y = CGAL::to_double(originObj.y());
												double cm_z = CGAL::to_double(originObj.z());
												btVector3 cmVector(cm_x, cm_y, cm_z);
												dobj.rotationAngle = dynamicObject->rotationAngle;
												dobj.rotationAxis = dynamicObject->rotationAxis;

												//Position
												btTransform pt(btQuaternion(getBtFromLVector3(dobj.rotationAxis),
																dynamicObject->rotationAngle),
																getBtFromLVector3(dynamicObject->position));
												btVector3 rotatedPosition = pt * cmVector;
												btVector3 initTranslation(rotatedPosition.getX(), rotatedPosition.getY(), rotatedPosition.getZ());

												//Add a translation on Y axis of 12.0
												dobj.position = getLFromBtVector3(initTranslation);

												//TODO: study the law for result linearVelocity and angularVelocity
												dobj.linearVelocity = dynamicObject->linearVelocity;
												dobj.angularVelocity = dynamicObject->angularVelocity;
												//dobj.angularVelocity = LVector3(0.0, 0.0, 0.0);

												dobj.isDynamic = true;
												dobj.statusInWorld = 0;

												//Add dobj to dynamicObjectsInWorld
												DynamicObject* ptrNewDynamicObject = new DynamicObject(dobj);
												dynamicObjectsInWorld.push_back(ptrNewDynamicObject);
												newObjectIntoDynamicObjectsInWorld = true;

												//Load here the dynamicObject->staticObject if use_memory_for_meshes
												if (LarmorPhysx::ConfigManager::use_memory_for_meshes)
												{
													//load staticObject from file
													ptrNewDynamicObject->staticObject = so;
												}


											}

											//Remove the old object from the scene changing the statusInWorld status
											std::cout << " Remove DynamicObject idObj: " << dynamicObject->idObj << std::endl;
											dynamicObject->statusInWorld = 2;
											//Bullet Physic remove from the world
											m_dynamicsWorld->removeRigidBody(body);

											//TODO: remove bullet btGImpactMeshShape and bTriangleMesh created with new
											//delete motionState;
											//m_dynamicsWorld->removeCollisionObject(collisionObj);
											//delete collisionObj;

										}
										else
										{
											std::cout << " ******* NOT SHATTERED numShatterPieces < 2 " << std::endl;
										}
									}
								}
							}

						}
					} //For numObjects


					//if some objects have been added to the scene
					if (newObjectIntoDynamicObjectsInWorld)
					{
						//remove the objects with statusInWorld == 2 from dynamicObjectsInWorld
						//descend visit to remove the issue for the erased element
						DynamicObjectPtrVector::iterator dynamicObjectPtrVectorIter;
						for(dynamicObjectPtrVectorIter = dynamicObjectsInWorld.end() -1;
								dynamicObjectPtrVectorIter >= dynamicObjectsInWorld.begin();
								--dynamicObjectPtrVectorIter)
						{
							if (dynamicObjectPtrVectorIter->statusInWorld == 2)
							{
								dynamicObjectsInWorld.erase(dynamicObjectPtrVectorIter);
							}
						}

						//Add newDynamicObjectsAdded to the world
						addDynamicObjectsToWorld();

					}

				} //if LarmorPhysx::ConfigManager::voronoi_shatter

			}


			void PhysicsCore::applyExternalForces()
			{
				//Apply here the external forces that change the world
			}


			void PhysicsCore::destroyWorld()
			{
				//TODO: at the end
			}


			void PhysicsCore::loadStartFrame(int startIdFrame)
			{
				stepFrame = LarmorPhysx::loadFrame(startIdFrame);
			}


			//TODO: use ConfigManager::use_simplified_meshes_for_bullet
			void PhysicsCore::addDynamicObjectsToWorld()
			{
				//Loop the dynamicObjectsInWorld vector for all the objects, building the objects into the world
				DynamicObjectPtrVector::iterator dynamicObjectPtrVectorIter;
				for(dynamicObjectPtrVectorIter = dynamicObjectsInWorld.begin();
						dynamicObjectPtrVectorIter != dynamicObjectsInWorld.end();
						++dynamicObjectPtrVectorIter)
				{
					//std::cout << "DynamicObject building in the world..." << std::endl;

					if (dynamicObjectPtrVectorIter->statusInWorld == 0)
					{
						//Load staticObject if not use_memory_for_meshes
						if (! LarmorPhysx::ConfigManager::use_memory_for_meshes)
						{
							//Load from file
							dynamicObjectPtrVectorIter->staticObject = LarmorPhysx::loadStaticObject( dynamicObjectPtrVectorIter->idObj );
						}
						//else dynamicObjectPtrVectorIter->staticObject should be already in memory

						//Update idGroupCounter
						if (dynamicObjectPtrVectorIter->idGroup > idGroupCounter)
						{
							idGroupCounter = dynamicObjectPtrVectorIter->idGroup;
						}

						//Point originObj = staticObject.meshData.second;
						//double cm_x = CGAL::to_double(originObj.x());
						//double cm_y = CGAL::to_double(originObj.y());
						//double cm_z = CGAL::to_double(originObj.z());
						//btVector3 objOrigin(cm_x, cm_y, cm_z);
						//btVector3 objInertia(getBtFromLVector3(staticObject.inertia));
						btVector3 objInertia(dynamicObjectPtrVectorIter->staticObject.inertia.x, dynamicObjectPtrVectorIter->staticObject.inertia.y, dynamicObjectPtrVectorIter->staticObject.inertia.z);
						double objMass = dynamicObjectPtrVectorIter->staticObject.mass;
						if (!dynamicObjectPtrVectorIter->isDynamic)
						{
							objInertia = btVector3(0.0, 0.0, 0.0);
							objMass = 0.0;
						}

						btTriangleMesh *triMeshTriangle = new  btTriangleMesh();

						std::list<Triangle>::iterator triangleIter;
						for(triangleIter = dynamicObjectPtrVectorIter->staticObject.meshData.first.first.begin();
								triangleIter != dynamicObjectPtrVectorIter->staticObject.meshData.first.first.end();
								++triangleIter)
						{
							Triangle t = *triangleIter;
							btVector3 tbtv[3];

							for (int i = 0; i < 3; ++i)
							{
								Point tv = t.vertex(i);
								double tx = CGAL::to_double(tv.x());
								double ty = CGAL::to_double(tv.y());
								double tz = CGAL::to_double(tv.z());
								tbtv[i] = btVector3(tx, ty, tz);
							}

							triMeshTriangle->addTriangle(tbtv[0], tbtv[1], tbtv[2]);
						}

						btGImpactMeshShape * trimesh = new btGImpactMeshShape(triMeshTriangle);
						trimesh->setMargin(0.0); //usare anche con  0.07 0.02 0.00f
						trimesh->updateBound();

						btTransform startTransform;
						startTransform.setIdentity();
						startTransform.setOrigin(getBtFromLVector3(dynamicObjectPtrVectorIter->position));
						startTransform.setRotation(btQuaternion(getBtFromLVector3(dynamicObjectPtrVectorIter->rotationAxis), dynamicObjectPtrVectorIter->rotationAngle));
						btDefaultMotionState* objMotionState = new btDefaultMotionState(startTransform);

						btRigidBody::btRigidBodyConstructionInfo cInfo(objMass, objMotionState, trimesh, objInertia);
						// READ: http://bulletphysics.org/mediawiki-1.5.8/index.php/BtContactSolverInfo#Resting_contact_restitution_threshold
						//cInfo.m_restitution = 1.0; //Non per mattoni uno sull'altro!
						//cInfo.m_friction = 0.0f;
						//cInfo.m_linearDamping = 0.2;
						//cInfo.m_angularDamping = 0.2;

						btRigidBody* body = new btRigidBody(cInfo);
						body->setContactProcessingThreshold(0.0);
						//TODO: test senza usare DISABLE_DEACTIVATION: come si comportano gli oggetti a riposo, continuano a saltare?
						if (LarmorPhysx::ConfigManager::disable_deactivation)
						{
							body->setActivationState(DISABLE_DEACTIVATION);
						}
						body->setLinearVelocity(getBtFromLVector3(dynamicObjectPtrVectorIter->linearVelocity));
						body->setAngularVelocity(getBtFromLVector3(dynamicObjectPtrVectorIter->angularVelocity));

						//Set DynamicObject pointer into the user pointer of body
						body->setUserPointer(&(*dynamicObjectPtrVectorIter));

						//enable custom material callback with ContactAddedCallback
						body->setCollisionFlags(body->getCollisionFlags()|btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

						//Add to the world
						m_dynamicsWorld->addRigidBody(body);

						//Update the statusInWorld status
						dynamicObjectPtrVectorIter->statusInWorld = 1;

						//clear dynamicObjectPtrVectorIter->staticObject.meshData if not use_memory_for_meshes
						if (! LarmorPhysx::ConfigManager::use_memory_for_meshes)
						{
							//Populate dynamicObjectVectorIter->staticObject with clear meshDatas for memory performance
							dynamicObjectPtrVectorIter->staticObject.meshData = TranslatedMeshData();
						}
					}
				}
			}


			void PhysicsCore::copyDynamicObjectsInWorldIntoFrame()
			{
				//Clear stepFrame.dynamicObjects
				stepFrame.dynamicObjects.clear();

				//Loop the dynamicObjectsInWorld vector for all the objects
				DynamicObjectPtrVector::iterator dynamicObjectPtrVectorIter;
				for(dynamicObjectPtrVectorIter = dynamicObjectsInWorld.begin();
						dynamicObjectPtrVectorIter != dynamicObjectsInWorld.end();
						++dynamicObjectPtrVectorIter)
				{
					DynamicObject dynamicObject = *dynamicObjectPtrVectorIter;
					stepFrame.dynamicObjects.push_back(dynamicObject);
				}
			}


			///User can override this material combiner by implementing gContactAddedCallback
			//  and setting body0->m_collisionFlags |= btCollisionObject::customMaterialCallback
			inline btScalar	calculateCombinedFriction(float friction0, float friction1)
			{
				//std::cout << "PhysicsCore::calculateCombinedFriction" << std::endl;
				btScalar friction = friction0 * friction1;

				const btScalar MAX_FRICTION  = 10.f;

				if (friction < -MAX_FRICTION)
					friction = -MAX_FRICTION;

				if (friction > MAX_FRICTION)
					friction = MAX_FRICTION;

				return friction;
			}

			inline btScalar	calculateCombinedRestitution(float restitution0, float restitution1)
			{
				//std::cout << "PhysicsCore::calculateCombinedRestitution" << std::endl;
				return restitution0 * restitution1;
			}

			//This seems work only for the mesh and plane collision
			bool customMaterialCombinerCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1)
			{
				//std::cout << "PhysicsCore::customMaterialCombinerCallback" << std::endl;

				int idObj0 = 0;
				int idObj1 = 0;
				int idGroup0 = 0;
				int idGroup1 = 0;
				const btCollisionObject* collisionObj0 = colObj0Wrap->getCollisionObject();
				const btRigidBody* body0 = btRigidBody::upcast(collisionObj0);
				if (body0->getUserPointer())
				{
					DynamicObject* dynamicObject0 = (DynamicObject*)(body0->getUserPointer());
					idObj0 = dynamicObject0->idObj;
					idGroup0 = dynamicObject0->idGroup;
				}
				const btCollisionObject* collisionObj1 = colObj1Wrap->getCollisionObject();
				const btRigidBody* body1 = btRigidBody::upcast(collisionObj1);
				if (body0->getUserPointer())
				{
					DynamicObject* dynamicObject1 = (DynamicObject*)(body1->getUserPointer());
					idObj1 = dynamicObject1->idObj;
					idGroup1 = dynamicObject1->idGroup;
				}

				//std::cout << "PhysicsCore::customMaterialCombinerCallback: " << idObj0 << ", " << idObj1 << " : "
				//		<< cp.m_combinedRestitution << " : " << cp.m_combinedFriction << std::endl;

				//Restitution 1 only with objects (idObj == 0 for the ground)
				if ((idObj0 != 0) && (idObj1 != 0))
				{
					if (idGroup0 != idGroup1)
					{
						cp.m_combinedRestitution = 0.8; //0.8
						cp.m_combinedFriction = 1.0;
					}
				}
				//cp.m_combinedFriction = 0.0;
				/*
				float friction0 = colObj0Wrap->getCollisionObject()->getFriction();
				float friction1 = colObj1Wrap->getCollisionObject()->getFriction();
				float restitution0 = colObj0Wrap->getCollisionObject()->getRestitution();
				float restitution1 = colObj1Wrap->getCollisionObject()->getRestitution();

				if (colObj0Wrap->getCollisionObject()->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
				{
					friction0 = 1.0;//partId0,index0
					restitution0 = 0.f;
				}
				if (colObj1Wrap->getCollisionObject()->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
				{
					if (index1&1)
					{
						friction1 = 1.0f;//partId1,index1
					} else
					{
						friction1 = 0.f;
					}
					restitution1 = 0.f;
				}

				cp.m_combinedFriction = LarmorPhysx::calculateCombinedFriction(friction0,friction1);
				cp.m_combinedRestitution = LarmorPhysx::calculateCombinedRestitution(restitution0,restitution1);
				*/

				//this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
				return true;
			}


			//double tempMaxImpulse = 0.0;
			void preTickCallbackLog(btDynamicsWorld *world, btScalar timeStep)
			{
				std::cout << "Sub Step: " << timeStep << std::endl;

				#ifdef SHOW_NUM_DEEP_PENETRATIONS
					printf("gNumDeepPenetrationChecks = %d\n",gNumDeepPenetrationChecks);
					printf("gNumSplitImpulseRecoveries= %d\n",gNumSplitImpulseRecoveries);
					printf("gNumGjkChecks= %d\n",gNumGjkChecks);
				#endif

				//New part study collision
				int numManifolds = world->getDispatcher()->getNumManifolds();
				#ifdef LOG_COLLISION_CALLBACK
					std::cout << " numManifolds: " << numManifolds << std::endl;
				#endif
				for (int manifoldIndex = 0; manifoldIndex < numManifolds; manifoldIndex++)
				{
					btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(manifoldIndex);
					//btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
					//btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

					int numContacts = contactManifold->getNumContacts();
					#ifdef LOG_COLLISION_CALLBACK
						std::cout << " numContacts: " << numContacts << std::endl;
					#endif

					if (numContacts > 0)
					{

						btScalar minDistance = 1e30f;
						btScalar totalImpact = 0.0f;
						int minIndex = -1;
						for (int contactIndex = 0; contactIndex < numContacts; contactIndex++)
						{
							if (minDistance >contactManifold->getContactPoint(contactIndex).getDistance())
							{
								minDistance = contactManifold->getContactPoint(contactIndex).getDistance();
								minIndex = contactIndex;
							}

							totalImpact +=contactManifold->getContactPoint(contactIndex).m_appliedImpulse;

							//Dump contact point
							btManifoldPoint& pt = contactManifold->getContactPoint(contactIndex);
							//btVector3 ptA = pt.getPositionWorldOnA();
							//btVector3 ptB = pt.getPositionWorldOnB();
							btVector3 ptA = pt.m_localPointA;
							btVector3 ptB = pt.m_localPointB;
							#ifdef LOG_COLLISION_CALLBACK
								printf("Contact points: (%f, %f, %f) (%f, %f, %f)\n", ptA.getX(), ptA.getY(), ptA.getZ(), ptB.getX(), ptB.getY(), ptB.getZ());
							#endif
						}
						#ifdef LOG_COLLISION_CALLBACK
							std::cout << " minDistance: " << minDistance << std::endl;
							std::cout << " minIndex: " << minIndex << std::endl;
							std::cout << " totalImpact: " << totalImpact << std::endl;
						#endif

						if ((minDistance <= 0.0) && (minIndex != -1) )
						{

							btCollisionObject* colObj0 = (btCollisionObject*)contactManifold->getBody0();
							btCollisionObject* colObj1 = (btCollisionObject*)contactManifold->getBody1();
							//	int tag0 = (colObj0)->getIslandTag();
							//  int tag1 = (colObj1)->getIslandTag();
							btRigidBody* body0 = btRigidBody::upcast(colObj0);
							btRigidBody* body1 = btRigidBody::upcast(colObj1);

							if (body0 && body1)
							{
								//if (!colObj0->isStaticOrKinematicObject() && !colObj1->isStaticOrKinematicObject())
								{
									//if (body0->checkCollideWithOverride(body1))
									{
										// Read http://bulletphysics.org/Bullet/BulletFull/classbtManifoldPoint.html
										btScalar impulse = contactManifold->getContactPoint(minIndex).m_appliedImpulse;
										btVector3 contactPosWorld = contactManifold->getContactPoint(minIndex).m_positionWorldOnA;
										btVector3 contactPosA = contactManifold->getContactPoint(minIndex).m_localPointA;
										btVector3 contactPosB = contactManifold->getContactPoint(minIndex).m_localPointB;
										btVector3 normalA = contactManifold->getContactPoint(minIndex).m_normalWorldOnB;
										btVector3 normalB = -normalA;
										btVector3 impulseVectorA = impulse * normalA;
										btVector3 impulseVectorB = impulse * normalB;
										#ifdef LOG_COLLISION_CALLBACK
											printf("Contact in: (%f, %f, %f) %f\n", contactPosWorld.getX(), contactPosWorld.getY(), contactPosWorld.getZ(), impulse );
											printf("impulseVector: (%f, %f, %f)\n", impulseVectorA.getX(), impulseVectorA.getY(), impulseVectorA.getZ() );
										#endif

										//Add to collisionPointVector
										if (impulse > 0.0)
										{
											CollisionPoint cpA;
											cpA.idObj1 = 0;
											cpA.idObj2 = 0;
											int objIdGroup1 = 0;
											int objIdGroup2 = 0;
											if (body0->getUserPointer())
											{
												DynamicObject* dynamicObject0 = (DynamicObject*)(body0->getUserPointer());
												cpA.idObj1 = dynamicObject0->idObj;
												objIdGroup1 = dynamicObject0->idGroup;
											}
											if (body1->getUserPointer())
											{
												DynamicObject* dynamicObject1 = (DynamicObject*)(body1->getUserPointer());
												cpA.idObj2 = dynamicObject1->idObj;
												objIdGroup2 = dynamicObject1->idGroup;
											}
											cpA.contactPoint = getLFromBtVector3(contactPosA);
											cpA.forceModule = impulse;
											cpA.collisionDirection = getLFromBtVector3(impulseVectorA);
											#ifdef LOG_COLLISION_CALLBACK
												std::cout << " Collision objects: " << cpA.idObj1 << ", " << cpA.idObj2 << std::endl;
											#endif

											CollisionPoint cpB;
											cpB.idObj1 = cpA.idObj2;
											cpB.idObj2 = cpA.idObj1;
											cpB.contactPoint = getLFromBtVector3(contactPosB);
											cpB.forceModule = impulse;
											cpB.collisionDirection = getLFromBtVector3(impulseVectorB);

											//This it patchs for wall of bricks
											//if( objIdGroup1 != objIdGroup2 )
											{
													mutexCollisionPointVector.lock();
													collisionPointVector.push_back(cpA);
													collisionPointVector.push_back(cpB);
													mutexCollisionPointVector.unlock();
											}

											//if (impulse > tempMaxImpulse)
											//{
											//	tempMaxImpulse = impulse;
											//	std::cout << " ******************** MAX IMPULSE: " << tempMaxImpulse << std::endl;
											//}

										}
									}
								}
							}
						}
					}
				}
				//End part study collision

			}


}



