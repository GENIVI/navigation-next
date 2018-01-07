/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------

    @defgroup TestMyPlaces_c Unit Tests for PAL My Places

    Unit Tests for PAL My Places

    This file contains all unit tests for the PAL My Places component
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testmyplaces.h"
#include "palrecents.h"
#include "palfavorites.h"
#include "PlatformUtil.h"

/*! Add all your test functions here
@return None
*/

static void PrepareMyPlaces(PAL_MyPlace* myPlace, PAL_MyPlace* myPlace2, PAL_MyPlace* myPlace3, PAL_MyPlace* myPlace4);

static void TestRecents(void)
{
    PAL_Error     err      = PAL_Ok;
    PAL_Instance* pal      = NULL;
    PAL_Recents*  instance = NULL;
    PAL_MyPlace   myPlace  = {0};
    PAL_MyPlace   myPlace2 = {0};
    PAL_MyPlace   myPlace3 = {0};
    PAL_MyPlace   myPlace4 = {0};
    PAL_MyPlace*  myPlace5 = NULL;
    PAL_MyPlace   myPlace6 = {0};
    int myPlaceCount       = 0;
    int id                 = 0;

    pal = PAL_CreateInstance();
    CU_ASSERT_FATAL(pal != NULL);

    err = PAL_RecentsCreate(pal, &instance);
    CU_ASSERT(err == PAL_Ok);

    PrepareMyPlaces(&myPlace, &myPlace2, &myPlace3, &myPlace4);

    err = PAL_RecentsAdd(instance, &myPlace);
    CU_ASSERT(err == PAL_Ok);
    err = PAL_RecentsAdd(instance, &myPlace2);
    CU_ASSERT(err == PAL_Ok);
    err = PAL_RecentsAdd(instance, &myPlace3);
    CU_ASSERT(err == PAL_Ok);
    err = PAL_RecentsAdd(instance, &myPlace4);
    CU_ASSERT(err == PAL_Ok);

    err = PAL_RecentsGet(instance, "McDonald's", &myPlace5, &myPlaceCount, PAL_ST_ByNone, 33.623125, -117.660313);
    CU_ASSERT(err == PAL_Ok);

    id = 4;
    err = PAL_RecentsGetPlaceById(instance, id, &myPlace6);
    CU_ASSERT(err == PAL_Ok);

    err = PAL_RecentsClear(instance);
    CU_ASSERT(err == PAL_Ok);

    err = PAL_RecentsDestroy(instance);
    CU_ASSERT(err == PAL_Ok);

    nsl_free(myPlace5);
    myPlace5 = NULL;

    PAL_Destroy(pal);

    return;
}

static void TestFavorites(void)
{
    PAL_Error      err      = PAL_Ok;
    PAL_Instance*  pal      = NULL;
    PAL_Favorites* instance = NULL;
    PAL_MyPlace    myPlace  = {0};
    PAL_MyPlace    myPlace2 = {0};
    PAL_MyPlace    myPlace3 = {0};
    PAL_MyPlace    myPlace4 = {0};
    PAL_MyPlace*   myPlace5 = NULL;
    PAL_MyPlace    myPlace6 = {0};
    int myPlaceCount        = 0;
    int id                  = 0;
    nb_boolean isFavorite   = FALSE;

    pal = PAL_CreateInstance();
    CU_ASSERT_FATAL(pal != NULL);

    err = PAL_FavoritesCreate(pal, &instance);
    CU_ASSERT(err == PAL_Ok);

    PrepareMyPlaces(&myPlace, &myPlace2, &myPlace3, &myPlace4);

    err = PAL_FavoritesAdd(instance, &myPlace);
    CU_ASSERT(err == PAL_Ok);
    err = PAL_FavoritesAdd(instance, &myPlace2);
    CU_ASSERT(err == PAL_Ok);
    err = PAL_FavoritesAdd(instance, &myPlace3);
    CU_ASSERT(err == PAL_Ok);
    err = PAL_FavoritesAdd(instance, &myPlace4);
    CU_ASSERT(err == PAL_Ok);

    err = PAL_FavoritesGet(instance, "Ralphs", &myPlace5, &myPlaceCount, PAL_ST_ByNone, 0, 0);
    CU_ASSERT(err == PAL_Ok);

    id = 4;
    err = PAL_FavoritesGetFavoriteByID(instance, id, &myPlace6);
    CU_ASSERT(err == PAL_Ok);

    id = 3;
    isFavorite = PAL_FavoritesIsFavorite(instance, id);
    CU_ASSERT(isFavorite == TRUE);

    err = PAL_FavoritesClear(instance);
    CU_ASSERT(err == PAL_Ok);

    err = PAL_FavoritesDestroy(instance);
    CU_ASSERT(err == PAL_Ok);

    nsl_free(myPlace5);
    myPlace5 = NULL;

    PAL_Destroy(pal);

    return;
}

void PrepareMyPlaces(PAL_MyPlace* myPlace, PAL_MyPlace* myPlace2, PAL_MyPlace* myPlace3, PAL_MyPlace* myPlace4)
{
    nsl_strlcpy(myPlace->name, "Ralphs", sizeof(myPlace->name));
    nsl_strlcpy(myPlace->url, "http://www.ralphs.com/", sizeof(myPlace->url));
    nsl_strlcpy(myPlace->imageUrl, "http://en.wikipedia.org/wiki/File:Ralphs.svg", sizeof(myPlace->imageUrl));
    nsl_strlcpy(myPlace->location.country, "USA", sizeof(myPlace->location.country));
    nsl_strlcpy(myPlace->location.city, "Aliso Viejo", sizeof(myPlace->location.city));
    nsl_strlcpy(myPlace->location.state, "CA", sizeof(myPlace->location.country));
    nsl_strlcpy(myPlace->phone.country, "1", sizeof(myPlace->phone.country));
    nsl_strlcpy(myPlace->phone.area, "949", sizeof(myPlace->phone.area));
    nsl_strlcpy(myPlace->phone.number, "362-3727", sizeof(myPlace->phone.number));
    nsl_strlcpy(myPlace->category.name, "Supermarket", sizeof(myPlace->category.name));
    myPlace->phone.type = PAL_MPPT_Primary;
    myPlace->type = PAL_MPT_Favorite;

    nsl_strlcpy(myPlace2->name, "McDonald's", sizeof(myPlace2->name));
    nsl_strlcpy(myPlace2->url, "http://www.mcdonalds.com/", sizeof(myPlace2->url));
    nsl_strlcpy(myPlace2->imageUrl, "http://en.wikipedia.org/wiki/File:McDonald%27s_Golden_Arches.svg", sizeof(myPlace2->imageUrl));
    nsl_strlcpy(myPlace2->location.country, "USA", sizeof(myPlace2->location.country));
    nsl_strlcpy(myPlace2->location.city, "Aliso Viejo", sizeof(myPlace2->location.city));
    nsl_strlcpy(myPlace2->location.state, "CA", sizeof(myPlace2->location.country));
    nsl_strlcpy(myPlace2->location.streetnum, "25192", sizeof(myPlace2->location.streetnum));
    nsl_strlcpy(myPlace2->location.street1, "Cabot Road", sizeof(myPlace2->location.street1));
    nsl_strlcpy(myPlace2->location.postal, "92653", sizeof(myPlace2->location.postal));
    nsl_strlcpy(myPlace2->phone.country, "1", sizeof(myPlace2->phone.country));
    nsl_strlcpy(myPlace2->phone.area, "949", sizeof(myPlace2->phone.area));
    nsl_strlcpy(myPlace2->phone.number, "462-0448", sizeof(myPlace2->phone.number));
    nsl_strlcpy(myPlace2->category.name, "Hamburger Restaurant", sizeof(myPlace2->category.name));
    myPlace2->phone.type = PAL_MPPT_Fax;
    myPlace2->location.latitude = 33.623125;
    myPlace2->location.longitude = -117.660313;
    myPlace2->type = PAL_MPT_Favorite;

    nsl_strlcpy(myPlace3->name, "Eiffel Tower", sizeof(myPlace3->name));
    nsl_strlcpy(myPlace3->url, "http://www.tour-eiffel.fr/", sizeof(myPlace3->url));
    nsl_strlcpy(myPlace3->imageUrl, "http://en.wikipedia.org/wiki/File:Tour_Eiffel_Wikimedia_Commons.jpg", sizeof(myPlace3->imageUrl));
    nsl_strlcpy(myPlace3->location.country, "FRA", sizeof(myPlace3->location.country));
    nsl_strlcpy(myPlace3->location.city, "Paris", sizeof(myPlace3->location.city));
    nsl_strlcpy(myPlace3->location.street1, "Avenue Gustave Eiffel", sizeof(myPlace3->location.street1));
    nsl_strlcpy(myPlace3->location.streetnum, "75007", sizeof(myPlace3->location.streetnum));
    nsl_strlcpy(myPlace3->phone.country, "33", sizeof(myPlace3->phone.country));
    nsl_strlcpy(myPlace3->phone.area, "892", sizeof(myPlace3->phone.area));
    nsl_strlcpy(myPlace3->phone.number, "70 12 39", sizeof(myPlace3->phone.number));
    nsl_strlcpy(myPlace3->category.name, "Tourist Attraction", sizeof(myPlace->category.name));
    myPlace3->phone.type = PAL_MPPT_Primary;
    myPlace3->type = PAL_MPT_Favorite;

    nsl_strlcpy(myPlace4->name, "McDonald's", sizeof(myPlace4->name));
    nsl_strlcpy(myPlace4->url, "http://www.mcdonalds.ru/", sizeof(myPlace4->url));
    nsl_strlcpy(myPlace4->imageUrl, "http://ru.wikipedia.org/wiki/%D0%A4%D0%B0%D0%B9%D0%BB:McDonald%27s_Golden_Arches.svg", sizeof(myPlace4->imageUrl));
    nsl_strlcpy(myPlace4->location.country, "RUS", sizeof(myPlace4->location.country));
    myPlace4->type = PAL_MPT_Favorite;
}

void
TestMyPlaces_AddAllTests(CU_pSuite pTestSuite)
{
    CU_add_test(pTestSuite, "TestRecents", &TestRecents);
    CU_add_test(pTestSuite, "TestFavorites", &TestFavorites);
}

/*! Add common initialization code here.

@return 0

@see TestClock_SuiteCleanup
*/
int
TestMyPlaces_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here.

@return 0

@see TestClock_SuiteSetup
*/
int
TestMyPlaces_SuiteCleanup()
{
    return 0;
}

/*! @} */