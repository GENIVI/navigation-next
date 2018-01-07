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

//
//  STMainViewController.m
//  searchTest
//
//  Created by Thomas Wu on 3/3/14.
//
//

#import "STMainViewController.h"
#import "initccc.h"
#import "STMainView.h"
#import "CommonTypes.h"
#import "nbsearchtypes.h"
#import "LocalPlaceManager.h"
#import "SearchHistoryManager.h"
#import "SingleSearchConfiguration.h"
#import "AnalyticsSearchEvent.h"

@interface STMainViewController ()
{
    CCCComponent*       m_pCCC;
    vector<SingleSearchRequest*>    m_requests;
}
- (void) onGotRecent:(const POISharedPtrVector&) POIs;
- (void) onGotFavorite:(const POISharedPtrVector&) POIs;
- (void) onRecentsUpdated:(const POISharedPtrVector&) POIs;
- (void) onFavoritesUpdated:(const POISharedPtrVector&) POIs;
- (void) onDBError:(int)errorCode;
@end

class STLocation : public LatLonPoint
{
public:
    STLocation(double la, double lo):lat(la),lon(lo){};
    virtual ~STLocation(){};

    virtual double GetLatitude() const {return lat;};
    virtual double GetLongitude() const {return lon;};
    double lat;
    double lon;
};

class LPOSListener:public LocalPlaceManager::GetPOIsListener
{
public:
    typedef enum
    {
        Recent = 0,
        Favorite = 1
    } ResultType;
    LPOSListener(id _delegate, ResultType _type = Favorite):delegate(_delegate), type(_type){};
    virtual ~LPOSListener(){};
    virtual void Success()
    {
        NSLog(@"LPOSListener Success");
    }
    virtual void Success(const POISharedPtrVector& POIs, const std::map<uint32, std::map<std::string, std::string> >& data)
    {
        switch (type)
        {
            case Recent:
                if ([delegate respondsToSelector:@selector(onGotRecent:)])
                {
                    [delegate onGotRecent:POIs];
                }
                break;
            case Favorite:
                if ([delegate respondsToSelector:@selector(onGotFavorite:)])
                {
                    [delegate onGotFavorite:POIs];
                }
                break;
            default:
                break;
        }
    };
    virtual void Error(LocalPlaceManagerError error)
    {
        if ([delegate respondsToSelector:@selector(onDBError:)])
        {
            [delegate onDBError:error];
        }
    };
private:
    id delegate;
    ResultType type;
};

class LPMListener:public LocalPlaceManager::Listener
{
public:
    LPMListener(id _delegate):delegate(_delegate){};
    virtual ~LPMListener(){};
    virtual void RecentsUpdated(const POISharedPtrVector& recents,const std::map<uint32, LocalPlaceUserData >& data)
    {
        if ([delegate respondsToSelector:@selector(onRecentsUpdated:)])
        {
            [delegate onRecentsUpdated:recents];
        }
    }
    virtual void FavoritesUpdated(const POISharedPtrVector& favorites,const std::map<uint32, LocalPlaceUserData >& data)
    {
        if ([delegate respondsToSelector:@selector(onFavoritesUpdated:)])
        {
            [delegate onFavoritesUpdated:favorites];
        }
    };
    virtual void Error(LocalPlaceManagerError error)
    {
        if ([delegate respondsToSelector:@selector(onDBError:)])
        {
            [delegate onDBError:error];
        }
    };
private:
    id delegate;
};

@implementation STMainViewController

- (void) onGotRecent:(const POISharedPtrVector&) POIs
{
    NSLog(@"Got Recent:%zu", POIs.size());
}
- (void) onGotFavorite:(const POISharedPtrVector&) POIs
{
    NSLog(@"Got Fav:%zu", POIs.size());
}
- (void) onRecentsUpdated:(const POISharedPtrVector&) POIs
{
    NSLog(@"recents updated");
}
- (void) onFavoritesUpdated:(const POISharedPtrVector&) POIs
{
    NSLog(@"favorites updated");
}
- (void) onDBError:(int)errorCode
{
    NSLog(@"error:%d", errorCode);
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
        m_pCCC = new CCCComponent();
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    if ([self.view isKindOfClass:[STMainView class]])
    {
        STMainView* view = (STMainView*)self.view;
        view.m_SearchButton.enabled = NO;
    }
    m_pCCC->AddListener((__bridge void*)self);
    m_pCCC->Initialize();
}

- (void)initCCCComplete
{
    NSLog(@"NBContext init complete");
    if ([self.view isKindOfClass:[STMainView class]])
    {
        STMainView* view = (STMainView*)self.view;
        view.m_SearchButton.enabled = YES;
        NSString *docsDir;
        NSArray *dirPaths;
        // Get the documents directory
        dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);



        docsDir = [dirPaths objectAtIndex:0];

        // Build the path to the database file
        NSString* RFdatabasePath = [[NSString alloc] initWithString: [docsDir stringByAppendingPathComponent: @"SKLocalPlace.sqlite"]];
        NSString* SHdatabasePath = [[NSString alloc] initWithString: [docsDir stringByAppendingPathComponent: @"SKSearchHistoryTest.sqlite"]];
        shared_ptr<LocalPlaceManager> lpm = LocalPlaceManager::GetInstance(m_pCCC->GetContext());
        lpm->RegisterListener(new LPMListener(self));
        lpm->InitializeDatabase([RFdatabasePath UTF8String]);
//        LocalPlaceManager::Initialize(m_pCCC->GetContext());
//        LocalPlaceManager::SetupDatabase(LocalPlaceManager::LLDBType_User, [databasePath UTF8String]);
//        LocalPlaceManager::RegisterListener(new LPMListener(self));

        shared_ptr<SearchHistoryManager> shm = SearchHistoryManager::GetInstance(m_pCCC->GetContext());
        shm->InitializeDatabase([SHdatabasePath UTF8String]);

        nbsearch::SearchHistoryData* shd = new SearchHistoryData("test text");
        shm->AddSearchHistoryData(shd);
        delete shd;
    }
}

- (IBAction)onSearchClicked:(id)sender
{
    NSLog(@"start search...");
    [self createSearchRequest];
    [self performSearch];
}

- (void)createSearchRequest
{
    using namespace nbsearch;
    STLocation* cL = new STLocation(35.00, 60.00);
    STLocation* sL = new STLocation(35.00, 60.00);
    STSSListener* listener = new STSSListener(self);
    SingleSearchConfiguration config;
    config.m_language = "en-US";
    config.m_resultStyleType = NB_SRST_SingleSearch;
//    config.m_resultStyleType = NB_SRST_Suggestion;
    config.m_sliceSize = 10;
    config.m_extendedConfiguration = (NB_POIExtendedConfiguration)( NB_EXT_WantCompactAddress|NB_EXT_WantExtendedAddress|NB_EXT_WantContentTypes|NB_EXT_WantStructuredHoursOfOperation);
    //NB_EXT_WantDistanceToUser|NB_EXT_WantResultDescription|NB_EXT_WantRelatedSearch|NB_EXT_WantSearchResultId|NB_EXT_WantIconID
    SingleSearchRequest* request = new SingleSearchRequest(m_pCCC->GetContext(),[@"coffee" UTF8String], "",cL,sL,listener,config);
    m_requests.push_back(request);
}

-(void)performSearch
{
    m_requests.at(m_requests.size() - 1)->StartRequest();
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
- (IBAction)onDBTestClicked:(id)sender
{
    shared_ptr<LocalPlaceManager> lpm = LocalPlaceManager::GetInstance(m_pCCC->GetContext());
    shared_ptr<SearchHistoryManager> shm = SearchHistoryManager::GetInstance(m_pCCC->GetContext());

    std::vector<shared_ptr<SearchHistoryData> > resultSH = shm->GetAllSearchHistoryData();

    POISharedPtrVector resultF;
    std::map<uint32, LocalPlaceUserData> dataF;
    lpm->GetFavorite(resultF, dataF);

    POISharedPtrVector resultR;
    std::map<uint32, LocalPlaceUserData> dataR;
    lpm->GetRecent(resultR, dataR);

    lpm->MoveFavorite(resultF.back().get(), 0, new LPOSListener(self));
    lpm->GetFavorite(resultF,dataF);

    lpm->RemoveRecentPOI(resultR[0].get(), new LPOSListener(self, LPOSListener::Recent));

    lpm->GetRecent(resultR, dataR);
}

#pragma mark-
#pragma mark STSSListener delegate

- (void)onCompleted:(const nbsearch::SingleSearchRequest *)request result:(const nbsearch::SingleSearchInformation *)result
{
    shared_ptr<LocalPlaceManager> lpm = LocalPlaceManager::GetInstance(m_pCCC->GetContext());
    shared_ptr<SearchHistoryManager> shm = SearchHistoryManager::GetInstance(m_pCCC->GetContext());
    NSLog(@"Completed  request:%p result:%p", request, result);

    switch (result->GetResultType()) {
        case nbsearch::SSIRT_POI:
            for (int i = 0; i < result->GetResultCount(); i ++)
            {
                nbsearch::POI const* poi = result->GetPOIAtIndex(i);
                const nbsearch::Place* place = poi->GetPlace();
                const nbsearch::POIContent* poicontent = poi->GetPOIContent();
                if(poicontent)
                {
                    const nbsearch::HoursOfOperation* hoo = poicontent->GetHoursOfOperation();
                    std::vector<POIKey> keys = poicontent->GetKeys();
                    for(POIKey key : keys)
                    {
                        std::vector<std::string> value;
                        poicontent->GetKeyValue(key, value);
                    }
                }
                double distance = poi->GetDistance();
                NSLog(@"POI %2d: name: %s distance:%f cAddress:%s city:%s", i, place->GetName().c_str(), distance, place->GetLocation()->GetCompactAddress()->GetLine1().c_str(), place->GetLocation()->GetCity().c_str());

                if (i%2)
                {
                    LocalPlaceUserData data;
                    data["favname"] = "";
                    lpm->AddFavoritePOI(poi,data, new LPOSListener(self, LPOSListener::Favorite));
                }
                else
                {
                    LocalPlaceUserData data;
                    lpm->AddRecentPOI(poi, data, new LPOSListener(self, LPOSListener::Recent));
                }

                nbsearch::Analytics::GetInstance(m_pCCC->GetContext())->AddEvent(poi->GetAnalyticsItem(), nbsearch::ActionOrigin_List, nbsearch::AnalyticsPlaceEvent_ShowDetail);
                nbsearch::Analytics::GetInstance(m_pCCC->GetContext())->UploadNow();
                NSLog(@"Analytics Event called at index %d.", i);
            }
            break;
        case nbsearch::SSIRT_SuggestionMatch:
            for (int i = 0; i < result->GetResultCount(); i++)
            {
                nbsearch::SuggestionMatch const* suggestion = result->GetSuggestionMatchAtIndex(i);
                NSLog(@"suggestion %2d: %s \t%s \t%s \t%f", i, suggestion->GetLine1().c_str(), suggestion->GetLine2().c_str(), suggestion->GetLine3().c_str(), suggestion->GetDistance());

                nbsearch::SearchHistoryData* shd = new SearchHistoryData(suggestion);
                shm->AddSearchHistoryData(shd);
                delete shd;
            }
            break;
        default:
            break;
    }
}

- (void)onError:(const nbsearch::SingleSearchRequest *)request code:(NB_Error)error
{
    NSLog(@"error  request:%p code:%d", request, error);
}

- (void)onProgressUpdated:(const nbsearch::SingleSearchRequest *)request progress:(int)progress
{
    NSLog(@"Progress  request:%p  %d%%", request, progress);
}
@end
