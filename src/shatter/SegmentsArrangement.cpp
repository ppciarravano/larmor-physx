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

#include "SegmentsArrangement.h"

//Return true if face1 has the same segments of face2
bool SegmentsArrangement::compareFacesList(std::list<Segment_2> face1, std::list<Segment_2> face2)
{

	if (face1.size() != face2.size())
	{
		return false;
	}

	if (face1.size() == 0)
	{
		return false;
	}

	bool result = true;
	std::list<Segment_2>::iterator face1SegsIter;
	face1SegsIter=face1.begin();
	Segment_2 firstSegFace1 = *face1SegsIter;

	std::list<Segment_2>::iterator face2SegsIter;
	for(face2SegsIter=face2.begin(); face2SegsIter != face2.end(); ++face2SegsIter)
	{
		Segment_2 segFace2 = *face2SegsIter;
		if (segFace2 == firstSegFace1) {
			break;
		}
	}

	if (face2SegsIter == face2.end())
	{
		//Primo elemento non trovato: quindi facce diverse
		return false;
	}

	for(face1SegsIter++; face1SegsIter != face1.end(); ++face1SegsIter)
	{
		Segment_2 segFace1 = *face1SegsIter;

		face2SegsIter++;
		if (face2SegsIter == face2.end())
		{
			face2SegsIter = face2.begin();
		}
		Segment_2 segFace2 = *face2SegsIter;

		//std::cout << "a: [" <<segFace1.start().x() << ", " << segFace1.start().y() << " ; ";
		//std::cout << segFace1.end().x() << ", " << segFace1.end().y() << "] " << std::endl;
		//std::cout << "b: [" <<segFace2.start().x() << ", " << segFace2.start().y() << " ; ";
		//std::cout << segFace2.end().x() << ", " << segFace2.end().y() << "] " << std::endl;

		if (segFace1 != segFace2)
		{
			result = false;
			break;
		}
	}
	return result;
}


//For Face and hole
//Cerca i segmenti gia' aggiunti cambiando il verso del segmento, se e' gia' presente lo elimina
//Puo funzionare sia per le faces sia per gli holes (senza utilizza pertanto getFaceInHalfedgeConstCirculatorHole)
std::list<Segment_2> SegmentsArrangement::getFaceInHalfedgeConstCirculatorFaceList(Arrangement_2::Ccb_halfedge_const_circulator &circ, bool counterClockwise)
{
	std::list<Segment_2> returnList;

	//FACE
	Arrangement_2::Ccb_halfedge_const_circulator currh = circ;
	do {
		Arrangement_2::Halfedge_const_handle  heh = currh;
		if (!heh->is_fictitious()) {

			Point_2 p1 = heh->source()->point();
			Point_2 p2 = heh->target()->point();
			//std::cout << "[" <<p1.x() << ", " << p1.y() << " ; ";
			//std::cout << p2.x() << ", " << p2.y() << "] " << std::endl;

			//creo un segment_2 e il suo opposto
			Segment_2 seg;
			Segment_2 segRepeat;
			if (counterClockwise) {
				seg = Segment_2(p2, p1);
				segRepeat = Segment_2(p1, p2);
			} else {
				seg = Segment_2(p1, p2);
				segRepeat = Segment_2(p2, p1);
			}
			//Aggiungo il segmento se non esiste o tolgo il suo ripetuto se l' ho gia' inserito
			if ((returnList.size() != 0) && (returnList.back() == segRepeat))	{
				returnList.pop_back();
				//std::cout << "ERASE BACK" << std::endl;
			} else if ((returnList.size() != 0) && (returnList.front() == segRepeat)) {
				returnList.pop_front();
				//std::cout << "ERASE FRONT" << std::endl;
			} else {
				if (counterClockwise) {
					returnList.push_front(seg);
				} else {
					returnList.push_back(seg);
				}
				//std::cout << "Add!" << std::endl;
			}

		}
		//else std::cout << "   [ ... ]   ";

	} while (++currh != circ);
	//std::cout << " END " << std::endl;

	/*
	//Dump list for test
	std::cout << "------------------\n";
	std::list<Segment_2>::iterator iter;
	for(iter=returnList.begin(); iter != returnList.end(); ++iter)
	{
		Segment_2 seg = *iter;
		std::cout << "S: [" <<seg.start().x() << ", " << seg.start().y() << " ; ";
		std::cout << seg.end().x() << ", " << seg.end().y() << "] " << std::endl;
	}
	std::cout << "***********************\n";
	 */

	return returnList;
}


/*
//CODICE UTILIZZATO NELLA PRIMA VERSIONE DEL SEGMENTSARRANGMENT QUANDO ANCORA NON UTILIZZAVA STD::LIST MA STD::MULTISET
//NON FUNZIONA BENE sPER VIA DI COME E' COSTRUITO IL classSegComp
//Return true if face1 has the same segments of face2
bool compareFacesSets(std::multiset<Segment_2,classSegComp> face1, std::multiset<Segment_2,classSegComp> face2)
{
	std::multiset<Segment_2,classSegComp>::iterator face1SegsIter;
	for(face1SegsIter=face1.begin(); face1SegsIter!= face1.end(); ++face1SegsIter)
	{
		Segment_2 segFace1 = *face1SegsIter;
		face2.erase(segFace1);
	}
	return face2.empty();
}
*/


/*
//CODICE UTILIZZATO NELLA PRIMA VERSIONE DEL SEGMENTSARRANGMENT QUANDO ANCORA NON UTILIZZAVA STD::LIST MA STD::MULTISET
//For Face and hole
//Cerca i segmenti gia' aggiunti cambiando il verso del segmento, se e' gia' presente lo elimina
//Puo funzionare sia per le faces sia per gli holes (senza utilizza pertanto getFaceInHalfedgeConstCirculatorHole)
std::multiset<Segment_2,classSegComp> getFaceInHalfedgeConstCirculatorFace(Arrangement_2::Ccb_halfedge_const_circulator &circ)
{
	std::multiset<Segment_2,classSegComp> returnSet;

	//FACE
	Arrangement_2::Ccb_halfedge_const_circulator currh = circ;
	do {
		Arrangement_2::Halfedge_const_handle  heh = currh;
		if (!heh->is_fictitious()) {

			Point_2 p1 = heh->source()->point();
			Point_2 p2 = heh->target()->point();
			//std::cout << "[" <<p1.x() << ", " << p1.y() << " ; ";
			//std::cout << p2.x() << ", " << p2.y() << "] " << std::endl;

			//creo un segment_2 e lo aggiungo
			//Segment_2 seg(p1, p2);
			//returnSet.insert(seg);

			//creo un segment_2 e il suo opposto
			Segment_2 seg(p1, p2);
			Segment_2 segRepeat(p2, p1);
			//Aggiungo il segmento se non esiste o tolgo il suo ripetuto se l' ho gia' inserito
			if (returnSet.count(segRepeat) == 0) {
				returnSet.insert(seg);
				//std::cout << "Add!" << std::endl;
			} else {
				returnSet.erase(segRepeat);
				//std::cout << "ERASE" << std::endl;
			}

		}
		//else std::cout << "   [ ... ]   ";

	} while (++currh != circ);
	//std::cout << " END " << std::endl;

	return returnSet;
}
*/


/*
//For hole (Ricalcolo l'arrangment sulla singola faccia per eliminare lati ripetuti)
// Un hole o face potrebbe avere segmenti che non formano facce chiuse o facce con segmenti sovrapposti
// quindi c'e' bisogno di ricalcolare l'arrangment sui segmenti
//NB:Le facce hanno segmenti ripetuti internamente, gli hole hanno segmenti ripetuti esternamente
//Affinche' questo metodo funzioni bene anche le le facce bisognerebbe creare un boundig box intorno alla faccia
//che si sta inserendo, in modo che quest'ultima diventi un hole a sua volta, e poi andare ad aggiungere
//solamente i segmenti dell'hole cosi generato
//NB: QUESTO METODO NON ERA NEPPURE UTILIZZATO NELLA PRIMA VERSIONE DEL SEGMENTSARRANGMENT QUANDO ANCORA NON UTILIZZAVA STD::LIST MA STD::MULTISE
std::multiset<Segment_2,classSegComp> getFaceInHalfedgeConstCirculatorHole(Arrangement_2::Ccb_halfedge_const_circulator &circ)
{
	std::multiset<Segment_2,classSegComp> returnSet;

	Arrangement_2 arr;

	//Aggiungo i segmenti all'arrangment
	Arrangement_2::Ccb_halfedge_const_circulator currh = circ;
	do {
		Arrangement_2::Halfedge_const_handle heh = currh;
		if (!heh->is_fictitious()) {
			Point_2 p1 = heh->source()->point();
			Point_2 p2 = heh->target()->point();
			//std::cout << "[" <<p1.x() << ", " << p1.y() << " ; ";
			//std::cout << p2.x() << ",  " << p2.y() << "] " << std::endl;

			//creo un segment_2 e lo aggiungo
			Segment_2 seg(p1, p2);
			insert(arr, seg);

		}
		//else std::cout << "   [ ... ]   ";

	} while (++currh != circ);
	//std::cout << " END " << std::endl;

	//Estraggo la faccia se c'e'
	Arrangement_2::Face_const_iterator faceConstIterator;
	Arrangement_2::Ccb_halfedge_const_circulator first_halfedgeConstIterator;
	Arrangement_2::Ccb_halfedge_const_circulator current_halfedgeConstIterator;
	Arrangement_2::Halfedge_const_handle halfedgeConstHandle;
	Arrangement_2::Face faceArrangement;
	Arrangement_2::Face faceArrangementToReturn;
	bool foundFace = false;
	//Ciclo su tutte le facce dell'arrangement
	for (faceConstIterator = arr.faces_begin();	faceConstIterator != arr.faces_end(); ++faceConstIterator) {
		faceArrangement = *faceConstIterator;
		//std::cout << "        HOLE FACE:" << std::endl;

		if (!faceConstIterator->is_unbounded()) {
			faceArrangementToReturn = *faceConstIterator;
			foundFace = true;
		}
	}

	if (foundFace) {

			Arrangement_2::Ccb_halfedge_const_circulator first_halfedgeConstIterator;
			Arrangement_2::Ccb_halfedge_const_circulator current_halfedgeConstIterator;
			Arrangement_2::Halfedge_const_handle halfedgeConstHandle;
			current_halfedgeConstIterator = first_halfedgeConstIterator = faceArrangementToReturn.outer_ccb();

			do {
				halfedgeConstHandle = current_halfedgeConstIterator;
				if (!halfedgeConstHandle->is_fictitious()) {

					//std::cout << "* ";
					Point_2 p1 = halfedgeConstHandle->source()->point();
					Point_2 p2 = halfedgeConstHandle->target()->point();
					//std::cout << "[" <<p1.x() << ", " << p1.y() << " ; ";
					//std::cout << p2.x() << " " << p2.y() << "] ";

					//creo un segment_2 e lo aggiungo
					Segment_2 seg(p1, p2);
					returnSet.insert(seg);

				}
				//else std::cout << "   [ ... ]   ";

				++current_halfedgeConstIterator;
			} while (current_halfedgeConstIterator != first_halfedgeConstIterator);
			//std::cout << " END " << std::endl;
	}
	return returnSet;
}
*/


void SegmentsArrangement::locatePoints(std::list<Point_2> points)
{
	#ifdef SEGMENTSARRANGEMENT_LOG
	std::cout << "Starting SegmentsArrangement::locatePoints..." << std::endl;
	#endif

	//Arrangment Batched Point-Location
	std::list<QueryResult> results;
	locate(arr, points.begin(), points.end(), std::back_inserter(results));

	//Presente come attributo dell'oggetto
	//MapQueryResult mapLocateQueryResult;

	std::list<QueryResult>::const_iterator resultsItererator;
	for (resultsItererator = results.begin(); resultsItererator != results.end(); ++resultsItererator)
	{
		mapLocateQueryResult.insert(*resultsItererator);
	}

	#ifdef SEGMENTSARRANGEMENT_LOG
	std::cout << "Ended SegmentsArrangement::locatePoints" << std::endl;
	#endif
}


#ifdef SEGMENTSARRANGEMENT_USE_POLYGON
bool SegmentsArrangement::isInArrangment(Point_2 point) {

	bool result = false;
	for (unsigned int faceIdx = 0; faceIdx < allFacesHolesVector.size(); faceIdx++)
	{
		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHolesVector.at(faceIdx);

		//Lancio la visita ricorsiva sul nodo root face di livello 0
		if (faceInfo->faceLevel == 0) {
			if (isInArrangmentRecursive(faceInfo, point) == 1)
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

//Ritorna
//0: non appartiene ad una faccia o ad un hole
//1: appartiene ad un faccia
//2: appartiene ad un hole
unsigned short int SegmentsArrangement::isInArrangmentRecursive(FaceHoleInfo *faceInfo, Point_2 &point) {

	unsigned short int result = 0;

	bool isInPolygon = (faceInfo->polygon).bounded_side(point) == CGAL::ON_BOUNDED_SIDE;
	//bool isInPolygon = (faceInfo->polygon).has_on_bounded_side(point);

	if (isInPolygon)
	{
		unsigned short int childsReturn = 0;
		for (unsigned int holeIdx = 0; holeIdx < faceInfo->holes.size(); holeIdx++)
		{
			//Estraggo dal vettore faceInfo->holes
			FaceHoleInfo *holeInfo = faceInfo->holes.at(holeIdx);

			//Lancio la visita ricorsiva sui figli
			unsigned short int childReturn = isInArrangmentRecursive(holeInfo, point);
			if ((childReturn > childsReturn))
			{
				childsReturn = childReturn;
			}
		}

		if (childsReturn == 0)
		{
			if (faceInfo->isFace)
			{
				return result = 1;
			}
			else
			{
				return result = 2;
			}
		}
		else
		{
			return childsReturn;
		}
	}

	return result;
}

#else

bool SegmentsArrangement::isInArrangment(Point_2 point) {

	bool result = false;
	MapQueryResult::iterator foundPair;
	if ( (foundPair = mapLocateQueryResult.find(point)) != mapLocateQueryResult.end())
	{
			Arrangement_2::Face_const_handle  fchp;
			//Cerco se il punto e' interno alla faccia utilizzando mapLocateQueryResult
			if (CGAL::assign(fchp, foundPair->second))
			{
				if ( !fchp->is_unbounded() )
				{
					for (unsigned int faceIdx = 0; faceIdx < allFacesHolesVector.size(); faceIdx++)
					{
						//Estraggo dal vettore FaceHoleInfo
						FaceHoleInfo *faceInfo = &allFacesHolesVector.at(faceIdx);
						Arrangement_2::Face_const_handle  faceHandle = faceInfo->faceArrangmentHandle;

						if (faceHandle == fchp)
						{
							if (faceInfo->isFace)
							{
								result = true;
							}
							break;
						}
					}
				}
			}
			//else
			//{
			//	std::cout << "WARNING: CGAL::assign not in a Face" << std::endl;
			//}
	}
	else
	{
		std::cout << "ERROR: POINT NOT FOUND IN mapLocateQueryResult!!" << std::endl;
	}
	return result;
}
#endif


#ifdef SEGMENTSARRANGEMENT_SET_FACELEVEL_AND_ISFACE
void SegmentsArrangement::recursiveVisitFacesHolesHierarchy(FaceHoleInfo *faceInfo, short int faceLevel) {

	//Incremento il livello
	faceLevel++;

	//Ciclo su tutte le facce di faceInfo cercando i buchi
	for (unsigned int holeIdx = 0; holeIdx < faceInfo->holes.size(); holeIdx++)
	{
		//std::cout << "      visita ricorsiva : " << holeIdx << " Level:" << faceLevel << std::endl;

		//Estraggo dal vettore faceInfo->holes
		FaceHoleInfo *holeInfo = faceInfo->holes.at(holeIdx);

		//Popolo il livello dei figli
		holeInfo->faceLevel = faceLevel;

		//Popolo isFace
		holeInfo->isFace = (faceLevel % 2) == 0;

		//Lancio la visita ricorsiva sui figli
		recursiveVisitFacesHolesHierarchy(holeInfo, faceLevel);

	}

}
#endif

/*
//TODO: Non funziona bene ancora e inoltre risente della scelta della precisione dello snap
std::vector<Segment_2> snapRounding(std::vector<Segment_2> outlineSegements)
{
	std::vector<Segment_2> result;

	Polyline_list_2 output_list;

	std::cout << "Start snap\n";

	// Generate an iterated snap-rounding representation, where the centers of
	// the hot pixels bear their original coordinates, using 5 kd trees:
	CGAL::snap_rounding_2<TraitsSnap, std::vector<Segment_2>::const_iterator, Polyline_list_2>
		(outlineSegements.begin(), outlineSegements.end(), output_list, 0.00001, true, false, 1);

	std::cout << "End snap\n";

	Polyline_list_2::const_iterator iter1;
	for (iter1 = output_list.begin(); iter1 != output_list.end(); ++iter1)
	{
		//std::cout << "Polyline number " << ++counter << ":\n";
		//Polyline_2::const_iterator iter2;
		//for (iter2 = iter1->begin(); iter2 != iter1->end(); ++iter2)
		//  std::cout << "    (" << iter2->x() << ":" << iter2->y() << ")\n";
		//TODO: bisogna inserire tutta la polyline come segmenti adiacenti
		KKPoint_2 ps = iter1->front();
		KKPoint_2 pe = iter1->back();
		if (ps != pe) {
			Segment_2 segSnap(ps, pe);
			result.push_back(segSnap);
		}
	}

	std::cout << "completed snap\n";

	return result;

}
*/

SegmentsArrangement::SegmentsArrangement()
{
	//std::cout << "Call SegmentsArrangement::SegmentsArrangement" << std::endl;
}

SegmentsArrangement::~SegmentsArrangement()
{
	//std::cout << "Call SegmentsArrangement::~SegmentsArrangement" << std::endl;

	/*
	for (unsigned int faceIdx = 0; faceIdx < allFacesHolesVector.size(); faceIdx++)
	{
		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHolesVector.at(faceIdx);

		delete faceInfo->parentFace;

		//TODO: ciclo su faceInfo->holes e delete puntatori
	}
	*/

}


std::vector<FaceHoleInfo> SegmentsArrangement::getFacesHolesHierarchy(std::vector<Segment_2> outlineSegements)
{
	//CGAL::Timer timer;
	//timer.start();

	//STEP 0
	//Si potrebbe utilizzare algoritmo di 2d Snap Rounding sui segmenti per eliminare eventuali segmenti non attaccati
	// Ma e' lento e inoltre risente della scelta della precisione dello snap
	//outlineSegements = snapRounding(outlineSegements);

	//Arrangment utilizzato:
	//Presente come attributo dell'oggetto
	//Arrangement_2 arr;

	//Struttura gerarchia facce buchi
	//Presente come attributo dell'oggetto
	//std::vector<FaceHoleInfo> allFacesHolesVector;

	//Add segments from outlineSegements for build the Arrangement_2
	insert(arr, outlineSegements.begin(), outlineSegements.end());

	/*
	//Metodo molto lento per aggiungere i segmenti, meglio usare quello precedente:	 insert(arr, InputIterator first, InputIterator last)
	//Add segments from outlineSegements for build the Arrangement_2
	for (unsigned int segIdx = 0; segIdx < outlineSegements.size(); segIdx++)
	{
		Segment_2 *segment = &outlineSegements.at(segIdx);
		insert(arr, *segment);
	}
	 */

	//std::cout << "ISVALID: " << arr.is_valid() << std::endl;

	// Print the arrangement dimensions:
	//std::cout << "V = " << arr.number_of_vertices() << ",  E = "
	//		<< arr.number_of_edges() << ",  F = " << arr.number_of_faces()
	//		<< std::endl;

	//STEP 1
	//std::cout << "STEP 1 getFacesHolesHierarchy: " << timer.time() << " seconds." << std::endl;
	//Ciclo su tutte le facce dell'arrangement per inserirle in allFacesVector
	for (Arrangement_2::Face_const_iterator faceConstIterator = arr.faces_begin();	faceConstIterator != arr.faces_end(); ++faceConstIterator) {

		Arrangement_2::Face faceArrangement = *faceConstIterator;
		//std::cout << "A*FACE:" << std::endl;
		//std::cout << "  Face is_fictitious: " << faceConstIterator->is_fictitious() << std::endl;

		if (faceConstIterator->is_unbounded()) {
			//std::cout << "    NOT FACE!! " << std::endl;
			continue;
		}

		//FACE
		//std::cout << "      FACEX: " << std::endl;
		Arrangement_2::Ccb_halfedge_const_circulator circ = faceArrangement.outer_ccb();
		//std::multiset<Segment_2,classSegComp> faceSegs = getFaceInHalfedgeConstCirculatorFace(circ); //PRIMA VERSIONE CON std::multiset
		std::list<Segment_2> faceSegs = getFaceInHalfedgeConstCirculatorFaceList(circ, false);

		//Inizializzo FaceHoleInfo, lo popolo e lo inserisco nel vettore allFacesHolesVector
		FaceHoleInfo faceInfo;
		faceInfo.segments = faceSegs;
		faceInfo.faceArrangement = *faceConstIterator;
		faceInfo.faceArrangmentHandle = faceConstIterator;
		faceInfo.isFace = true;
		faceInfo.faceLevel = 0;
		faceInfo.parentFace = NULL;
		#ifdef SEGMENTSARRANGEMENT_USE_POLYGON
			//Costruisco il poligono utilizzando i faceSegs
			std::list<Kernel_Point_2> polygonPoints;
			std::list<Segment_2>::iterator faceSegsIter;
			for(faceSegsIter=faceSegs.begin(); faceSegsIter!= faceSegs.end(); ++faceSegsIter)
			{
				Segment_2 segFace = *faceSegsIter;
				polygonPoints.push_back(segFace.start());
			}
			faceInfo.polygon = Polygon_2(polygonPoints.begin(), polygonPoints.end());
			if (!faceInfo.polygon.is_simple()) {
				std::cout << "WARNING: POLYGON IS NOT SIMPLE!!!" << std::endl;
			}
			//else {
			//	std::cout << "POLYGON IS SIMPLE!!!" << std::endl;
			//}
		#endif
		allFacesHolesVector.push_back(faceInfo);
	}

	//std::cout << "      FACE allFacesHolesVector.size: " << allFacesHolesVector.size() << std::endl;

	//STEP 2
	//std::cout << "STEP 2 getFacesHolesHierarchy: " << timer.time() << " seconds." << std::endl;
	//Ciclo su tutte le facce di allFacesHolesVector cercando i buchi
	for (unsigned int faceIdx = 0; faceIdx < allFacesHolesVector.size(); faceIdx++)
	{

		//std::cout << "      faceIdx : " << faceIdx << std::endl;

		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHolesVector.at(faceIdx);

		//Arrangement_2::Hole_iterator h_ite;
		//Arrangement_2::Ccb_halfedge_const_circulator circ;
		//Cerco tutti i buchi di una faccia
		for (Arrangement_2::Hole_iterator  h_ite = (faceInfo->faceArrangement.holes_begin()); h_ite != (faceInfo->faceArrangement.holes_end()); ++h_ite)
		{
			//std::cout << "    HOLEX: " << std::endl;
			Arrangement_2::Ccb_halfedge_const_circulator circ = *h_ite;

			//creo il set per i segmenti del buco
			//std::multiset<Segment_2,classSegComp> holeSegs = getFaceInHalfedgeConstCirculatorFace(circ); //PRIMA VERSIONE CON std::multiset
			std::list<Segment_2> holeSegs = getFaceInHalfedgeConstCirculatorFaceList(circ, true);

			//Se holeSegs non contiene segmenti passo la hole seguente se c'e'
			if (holeSegs.size() == 0)
			{
				continue;
			}

			//Ciclo su tutte le facce cercando quale faccia is equal al buco
			bool isFoundedHole = false;
			for (unsigned int hinnerFaceIdx = 0; hinnerFaceIdx < allFacesHolesVector.size(); hinnerFaceIdx++)
			{
				if (faceIdx != hinnerFaceIdx) {

					//printf("       hinnerFaceIdx: %d\n", hinnerFaceIdx);

					//Estraggo dal vettore FaceHoleInfo
					FaceHoleInfo *hinnerFaceInfo = &allFacesHolesVector.at(hinnerFaceIdx);

					//bool isHoleEqualsFace = compareFacesSets(holeSegs, hinnerFaceInfo->segments); //PRIMA VERSIONE CON std::multiset
					bool isHoleEqualsFace = compareFacesList(holeSegs, hinnerFaceInfo->segments);
					if (isHoleEqualsFace) {
						//Hole rappresentato da hinnerFaceInfo appartiene alla faccia faceInfo
						//std::cout << "        HOLE EQUALS FACE!" << std::endl;

						//Popolo i riferimenti per faceInfo.holes
						faceInfo->holes.push_back(hinnerFaceInfo);
						//Popolo il riferimenti per hinnerFaceInfo.parentFace
						hinnerFaceInfo->faceLevel = 1;
						hinnerFaceInfo->parentFace = faceInfo;

						isFoundedHole = true;

						//Esco dal ciclo for dato che ho trovato la faccia
						break;
					}
					//else
					//{
					//	std::cout << "        HOLE NOT EQUALS FACE!" << std::endl;
					//}
				}
			} //Close for on hinnerFaceIdx

			//Per il bug di un hole diviso in due:
			//
			//     ----------
			//     | -----  |
			//     | | | |  |
			//     | -----  |
			//     ----------
			if (!isFoundedHole)
			{

				//Se il buco non e' stato trovato corrispondente ad una faccia allora aggiungo il buco
				// alle facce e creo la referenza alla faccia a cui appartiene

				std::cout << "ATTENTION: HOLE DIDN'T FOUND!!!" << std::endl;

				/*
				//Inizializzo FaceHoleInfo e lo inserisco nel vettore allFacesHolesVector
				FaceHoleInfo holeInfo;
				holeInfo.segments = holeSegs;
				holeInfo.isFace = true;
				holeInfo.faceLevel = 1;
				holeInfo.parentFace = faceInfo;
				allFacesHolesVector.push_back(holeInfo);

				//Estraggo il puntatore dell'elemento che ho appena inserito
				FaceHoleInfo *insertedFaceInfo = &allFacesHolesVector.at(allFacesHolesVector.size() - 1);

				//Notare che questi due vettore sono diversi e non posso inserire direttamente &holeInfo
				//printf("puntatore 1: %p\n",  &holeInfo);
				//printf("puntatore 2: %p\n",  faceInfo);

				//Popolo i riferimenti per faceInfo.holes
				faceInfo->holes.push_back(insertedFaceInfo);

				//TODO: bisogna eliminare le facce interne ad alla faccia rappresentata da holeInfo
				// da fare di seguito
				*/

			}

		} //Close for on hole iterator
	} //Close for on faceIdx


	#ifdef SEGMENTSARRANGEMENT_SET_FACELEVEL_AND_ISFACE
	//STEP 3
	//std::cout << "STEP 3 getFacesHolesHierarchy: " << timer.time() << " seconds." << std::endl;
	//Ciclo su tutte le facce di allFacesHolesVector cercando le facce di livello 0, cioe' le faccie che non hanno genitore
	// Visita ricorsiva per andare a popolare isFace e faceLevel correttamente
	for (unsigned int faceIdx = 0; faceIdx < allFacesHolesVector.size(); faceIdx++)
	{
		//std::cout << "      ultima visita faceIdx : " << faceIdx << std::endl;

		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHolesVector.at(faceIdx);

		//if (faceInfo->parentFace == NULL) {
		if (faceInfo->faceLevel == 0) {
			//std::cout << "      FACCIA di primo livello " << faceIdx << std::endl;
			//Lancio la visita ricorsiva per popolare il campo faceInfo->isFace and faceInfo->faceLevel
			recursiveVisitFacesHolesHierarchy(faceInfo, faceInfo->faceLevel);
		}
	}
	#endif


	#ifdef SEGMENTSARRANGEMENT_FIND_SEEDS
	//STEP 4
	//std::cout << "STEP 4 getFacesHolesHierarchy: " << timer.time() << " seconds." << std::endl;
	//Cerco e individuo un seeds per ogni Holes, che non appartenga pero alle sue facce interne,
	// Per stare dentro ad hole e non alle facce interne, allora presa una linea che parte da quel punto trovato
	// e una direzione della linea, il numero delle intersezioni con tutti i segmenti delle facce deve essere dispari
	for (unsigned int faceIdx = 0; faceIdx < allFacesHolesVector.size(); faceIdx++)
	{
		//Estraggo dal vettore FaceHoleInfo
		FaceHoleInfo *faceInfo = &allFacesHolesVector.at(faceIdx);

		//Cerco i seed solo per gli hole, dato che per la mesh sono interessato solo a questi seed
		//Comunque se lascio commentato l'if seguente, trovo tutti i seed delle facce
		// Dovrei calcolare i seeds anche per le facce se voglio usare il controll per generare la mesh solo
		//  per le superfici chiuse: vedi commento in VoronoiMesher.cpp
		if (!faceInfo->isFace) {

			//holeAndFacesSegments conterra' i segmenti della faccia e dei buchi interni alla faccia di primo livello
			std::list<Segment_2> holeAndFacesSegments;

			//Aggiungo i segmenti della faccia
			//std::multiset<Segment_2,classSegComp>::iterator segmentIterFace; //PRIMA VERSIONE CON std::multiset
			std::list<Segment_2>::iterator segmentIterFace;
			for(segmentIterFace=faceInfo->segments.begin(); segmentIterFace != faceInfo->segments.end(); ++segmentIterFace)
			{
				Segment_2 seg = *segmentIterFace;
				holeAndFacesSegments.push_back(seg);
				//std::cout << "Added seg!" << std::endl;
			}

			//Ciclo gli holes della faccia e inserisco i segmenti di ogni hole
			for (unsigned int holeIdx = 0; holeIdx < faceInfo->holes.size(); holeIdx++)
			{
				//Estraggo dal vettore faceInfo->holes
				FaceHoleInfo *holeInfo = faceInfo->holes.at(holeIdx);

				//Aggiungo i segmenti della faccia
				//std::multiset<Segment_2,classSegComp>::iterator segmentIterHole; //PRIMA VERSIONE CON std::multiset
				std::list<Segment_2>::iterator segmentIterHole;
				for(segmentIterHole=holeInfo->segments.begin(); segmentIterHole != holeInfo->segments.end(); ++segmentIterHole)
				{
					Segment_2 seg = *segmentIterHole;
					holeAndFacesSegments.push_back(seg);
					//std::cout << "Added seg!" << std::endl;
				}
			}

			//Ora holeAndFacesSegments contiene tutti i segmenti della faccia e degli holes al suo interno
			//std::cout << "size: " << holeAndFacesSegments.size() << std::endl;

			//Ciclo tutti i segmenti della faccia faceInfo->segments e per ogni segmento genero un segmento "infinito"
			//perpependicolare al punto mediano e lo aggiungo ad holeAndFacesSegments;
			//ad ogni iterazione verifico i punti di intersezione e a fine interazione elimino il segmento aggiunto in coda alla lista
			//std::multiset<Segment_2,classSegComp>::iterator segmentIterFaceNew; //PRIMA VERSIONE CON std::multiset
			std::list<Segment_2>::iterator segmentIterFaceNew;
			for(segmentIterFaceNew=faceInfo->segments.begin(); segmentIterFaceNew != faceInfo->segments.end(); ++segmentIterFaceNew)
			{
				Segment_2 seg = *segmentIterFaceNew;

				//Trovo punto mediano del segmento utilizzando la funzione 2d di CGAL
				Point_2 segMidpoint = midpoint(seg.start(), seg.end());

				//Trovo segmento perpendicolare al punto mediano
				Line_2 segLine = seg.supporting_line();
				Line_2 segLinePerpendicular = segLine.perpendicular(segMidpoint);
				//TODO: generare in modo migliore il segmento perpendicolare da segLinePerpendicular
				Segment_2 segPerpendicular(segLinePerpendicular.point(-10000000), segLinePerpendicular.point(+10000000));

				//Aggiungo il segmento segPerpendicular in coda alla lista holeAndFacesSegments
				holeAndFacesSegments.push_back(segPerpendicular);

				//Compute all intersection points.
				std::vector<Point_2> intersectionPoints;
				CGAL::compute_intersection_points (holeAndFacesSegments.begin(), holeAndFacesSegments.end(),
							std::back_inserter(intersectionPoints));
				//std::cout << "compute_intersection_points: " << intersectionPoints.size() << std::endl;

				//Se il numero di punti trovati e' dispari, qualcosa non va, e' impossibile, pertanto
				//faccio un continue del ciclo for
				if ((intersectionPoints.size() % 2) == 1) {
					std::cout << "WARNING: ODD NUMBER OF INTERSECTION POINTS" << std::endl;

					holeAndFacesSegments.pop_back();
					continue; //next for iteration
				}

				//I punti in intersectionPoint sono in an ascending xy-lexicographic order
				//Pertanto tra questi deve essere presente certamente il punto segMidpoint
				//Cerco la sua posizione nella lista intersectionPoint
				//una volta trovata la posizione, scelgo il punto a destra o a sinistra a seconda
				//di quale meta' contiene un numero dispari di punti
				unsigned int idx = 0;
				bool isFoundedMidpoint = false;
				for (idx = 0; idx < intersectionPoints.size(); idx++)
				{
					Point_2 point = intersectionPoints.at(idx);
					if (point == segMidpoint) {
						//std::cout << "FOUNDED MIDPOINT: " << idx << std::endl;
						isFoundedMidpoint = true;
						break;
					}
				}

				if (isFoundedMidpoint) {

					//Se idx e' pari allora prendo il punto idx+1
					//se idx e' dispari allora prendo il punto idx-1
					Point_2 nextPoint;
					if ((idx % 2) == 0) {
						nextPoint = intersectionPoints.at(idx+1);
					} else {
						nextPoint = intersectionPoints.at(idx-1);
					}

					Point_2 seed =  midpoint(segMidpoint, nextPoint);

					//Se il punto seed e' stato trovato allora inserisco il punto in faceInfo->seed e break ciclo for
					faceInfo->seed = seed;

					break; //sicuramente in questo punto ho trovato il seed e posso uscire dal ciclo for

				} else {

					std::cout << "WARNING: MIDPOINT DIDN'T FOUND into intersectionPoints" << std::endl;

				}

				//Altrimenti rimuovo dalla coda della lista holeAndFacesSegments il segmento aggiunto inizialmente seg
				holeAndFacesSegments.pop_back();

			}
		}
	}
	#endif

	//timer.stop();
	//std::cout << "Construction took for getFacesHolesHierarchy: " << timer.time() << " seconds." << std::endl;

	//Simplification arrangment
	arrangmentSimplification(allFacesHolesVector);

	//Return the std::vector<FaceHoleInfo>
	return allFacesHolesVector;

}

std::vector<FaceHoleInfo> SegmentsArrangement::getAllFacesHolesVectorInObject()
{
	return allFacesHolesVector;
}

//Simplification arrangment: se piu' segmenti appartengono ad una stessa linea, ritorna un segmento unico che comprenda i segmenti
// usando http://www.cgal.org/Manual/latest/doc_html/cgal_manual/Kernel_23_ref/Function_orientation.html
void SegmentsArrangement::arrangmentSimplification(std::vector<FaceHoleInfo> &facesHolesVector)
{

	if (facesHolesVector.size() > 0)
	{
		for (unsigned int faceIdx = 0; faceIdx < facesHolesVector.size(); faceIdx++)
		{
			//Estraggo dal vettore FaceHoleInfo
			FaceHoleInfo *faceInfo = &facesHolesVector.at(faceIdx);
			std::list<Segment_2> newSegments;
			Point_2 prec = ((Segment_2)(*(faceInfo->segments.begin()))).start();
			bool canAddSegments = false;
			std::list<Segment_2>::iterator faceSegsIter;
			std::list<Segment_2>::iterator faceSegsIterStop;
			//std::cout << "FACE NUM. SEGS BEFORE SEMPLIFICATION: " << faceInfo->segments.size() << std::endl;
			for (unsigned int loop = 0; loop < 2; loop++)
			{
				for(faceSegsIter=faceInfo->segments.begin(); faceSegsIter!= faceInfo->segments.end(); ++faceSegsIter)
				{
					Segment_2 seg = *faceSegsIter;
					Point_2 start = seg.start();
					Point_2 end = seg.end();
					//std::cout << "[" <<seg.start().x() << ", " << seg.start().y() << " ; ";
					//std::cout << seg.end().x() << ", " << seg.end().y() << "] " << std::endl;
					//std::cout << "SEG_ORIENTATION: " << CGAL::orientation(prec, start, end) << std::endl;

					//Eseguito finche non si trova uno spigolo
					//if ((!canAddSegments) && (CGAL::orientation(prec, start, end) != CGAL::COLLINEAR))
					if ((!canAddSegments) &&
							( (CGAL::orientation(prec, start, end) != CGAL::COLLINEAR) ||
									( (Segment_2(prec, start).direction() != seg.direction()) && (prec != start)) ) )
					{
						//std::cout << "STEP 1" << std::endl;
						canAddSegments = true;
						prec = start;
						faceSegsIterStop = faceSegsIter;
						continue;
					}

					//Eseguito solo dopo che si e' trovato il primo spigolo
					//if (canAddSegments && (CGAL::orientation(prec, start, end) != CGAL::COLLINEAR))
					if (canAddSegments &&
							( (CGAL::orientation(prec, start, end) != CGAL::COLLINEAR) ||
									( (Segment_2(prec, start).direction() != seg.direction()) && (prec != start)) ) )
					{
						//std::cout << "STEP 2" << std::endl;
						//Aggiungo segmento prec-start
						Segment_2 segPrecStart(prec, start);
						newSegments.push_back(segPrecStart);
						prec = start;
					}

					if((loop == 1) && faceSegsIter == faceSegsIterStop)
					{
						break;
					}
				}
			}
			//Assegno a segments i segmenti trovati
			//std::cout << "SEMPLIFICATED SEGS: " << (faceInfo->segments.size() - newSegments.size()) << std::endl;
			faceInfo->segments = newSegments;
			//std::cout << "FACE NUM. SEGS AFTER SEMPLIFICATION: " << faceInfo->segments.size() << std::endl;
		}
	}

}

