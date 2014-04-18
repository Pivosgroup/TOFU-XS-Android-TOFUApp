import sys, datetime
import xbmc, xbmcgui
import contextmenu, infodialog
if sys.version_info < (2, 7):
    import simplejson
else:
    import json as simplejson

__addon__        = sys.modules[ "__main__" ].__addon__
__addonid__      = sys.modules[ "__main__" ].__addonid__
__addonversion__ = sys.modules[ "__main__" ].__addonversion__
__language__     = sys.modules[ "__main__" ].__language__
__cwd__          = sys.modules[ "__main__" ].__cwd__

ACTION_CANCEL_DIALOG = ( 9, 10, 92, 216, 247, 257, 275, 61467, 61448, )
ACTION_CONTEXT_MENU = ( 117, )
ACTION_OSD = ( 122, )
ACTION_SHOW_GUI = ( 18, )
ACTION_SHOW_INFO = ( 11, )

def log(txt):
    if isinstance (txt,str):
        txt = txt.decode("utf-8")
    message = u'%s: %s' % (__addonid__, txt)
    xbmc.log(msg=message.encode("utf-8"), level=xbmc.LOGDEBUG)

class GUI( xbmcgui.WindowXMLDialog ):
    def __init__( self, *args, **kwargs ):
        self.searchstring = kwargs[ "searchstring" ].replace('(', '[(]').replace(')', '[)]').replace('+', '[+]')
        log('script version %s started' % __addonversion__)
        self.nextsearch = False

    def onInit( self ):
        if self.searchstring == '':
            self._close()
        else:
            self.window_id = xbmcgui.getCurrentWindowDialogId()
            xbmcgui.Window(self.window_id).setProperty('GlobalSearch.SearchString', self.searchstring)
            self._hide_controls()
            if not self.nextsearch:
                self._parse_argv()
                if self.params == {}:
                    self._load_settings()
            self._reset_variables()
            self._init_variables()
            self._fetch_items()

    def _fetch_items( self ):
        if self.movies == 'true':
            self._fetch_movies()
        if self.tvshows == 'true':
            self._fetch_tvshows()
        if self.episodes == 'true':
            self._fetch_episodes()
        if self.musicvideos == 'true':
            self._fetch_musicvideos()
        if self.artists == 'true':
            self._fetch_artists()
        if self.albums == 'true':
            self._fetch_albums()
        if self.songs == 'true':
            self._fetch_songs()
        self._check_focus()

    def _hide_controls( self ):
        self.getControl( 119 ).setVisible( False )
        self.getControl( 129 ).setVisible( False )
        self.getControl( 139 ).setVisible( False )
        self.getControl( 149 ).setVisible( False )
        self.getControl( 159 ).setVisible( False )
        self.getControl( 169 ).setVisible( False )
        self.getControl( 179 ).setVisible( False )
        self.getControl( 189 ).setVisible( False )
        self.getControl( 198 ).setVisible( False )
        self.getControl( 199 ).setVisible( False )

    def _reset_controls( self ):
        self.getControl( 111 ).reset()
        self.getControl( 121 ).reset()
        self.getControl( 131 ).reset()
        self.getControl( 141 ).reset()
        self.getControl( 151 ).reset()
        self.getControl( 161 ).reset()
        self.getControl( 171 ).reset()
        self.getControl( 181 ).reset()

    def _parse_argv( self ):
        try:
            self.params = dict( arg.split( "=" ) for arg in sys.argv[ 1 ].split( "&" ) )
        except:
            self.params = {}
        self.movies = self.params.get( "movies", "" )
        self.tvshows = self.params.get( "tvshows", "" )
        self.episodes = self.params.get( "episodes", "" )
        self.musicvideos = self.params.get( "musicvideos", "" )
        self.artists = self.params.get( "artists", "" )
        self.albums = self.params.get( "albums", "" )
        self.songs = self.params.get( "songs", "" )

    def _load_settings( self ):
        self.movies = __addon__.getSetting( "movies" )
        self.tvshows = __addon__.getSetting( "tvshows" )
        self.episodes = __addon__.getSetting( "episodes" )
        self.musicvideos = __addon__.getSetting( "musicvideos" )
        self.artists = __addon__.getSetting( "artists" )
        self.albums = __addon__.getSetting( "albums" )
        self.songs = __addon__.getSetting( "songs" )

    def _reset_variables( self ):
        self.focusset= 'false'
        self.getControl( 190 ).setLabel( '[B]' + xbmc.getLocalizedString(194) + '[/B]' )

    def _init_variables( self ):
        self.fetch_seasonepisodes = 'false'
        self.fetch_albumssongs = 'false'
        self.fetch_songalbum = 'false'
        self.playingtrailer = 'false'
        self.getControl( 198 ).setLabel( '[B]' + __language__(32299) + '[/B]' )
        self.Player = MyPlayer()
        self.Player.gui = self

    def _fetch_movies( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(342) + '[/B]' )
        count = 0
        json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "VideoLibrary.GetMovies", "params": {"properties": ["title", "streamdetails", "genre", "studio", "year", "tagline", "plot", "plotoutline", "runtime", "fanart", "thumbnail", "file", "trailer", "playcount", "rating", "mpaa", "director", "writer"], "sort": { "method": "label" }, "filter": {"field":"title","operator":"contains","value":"%s"} }, "id": 1}' % self.searchstring)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('movies')):
            for item in json_response['result']['movies']:
                movie = item['title']
                count = count + 1
                director = " / ".join(item['director'])
                writer = " / ".join(item['writer'])
                fanart = item['fanart']
                path = item['file']
                genre = " / ".join(item['genre'])
                mpaa = item['mpaa']
                playcount = str(item['playcount'])
                plot = item['plot']
                outline = item['plotoutline']
                rating = str(round(float(item['rating']),1))
                starrating = 'rating%.1d.png' % round(float(rating)/2)
                runtime = str(int((item['runtime'] / 60.0) + 0.5))
                studio = " / ".join(item['studio'])
                tagline = item['tagline']
                thumb = item['thumbnail']
                trailer = item['trailer']
                year = str(item['year'])
                if item['streamdetails']['audio'] != []:
                    audiochannels = str(item['streamdetails']['audio'][0]['channels'])
                    audiocodec = str(item['streamdetails']['audio'][0]['codec'])
                else:
                    audiochannels = ''
                    audiocodec = ''
                if item['streamdetails']['video'] != []:
                    videocodec = str(item['streamdetails']['video'][0]['codec'])
                    videoaspect = float(item['streamdetails']['video'][0]['aspect'])
                    if videoaspect <= 1.4859:
                        videoaspect = '1.33'
                    elif videoaspect <= 1.7190:
                        videoaspect = '1.66'
                    elif videoaspect <= 1.8147:
                        videoaspect = '1.78'
                    elif videoaspect <= 2.0174:
                        videoaspect = '1.85'
                    elif videoaspect <= 2.2738:
                        videoaspect = '2.20'
                    else:
                        videoaspect = '2.35'
                    videowidth = item['streamdetails']['video'][0]['width']
                    videoheight = item['streamdetails']['video'][0]['height']
                    if videowidth <= 720 and videoheight <= 480:
                        videoresolution = '480'
                    elif videowidth <= 768 and videoheight <= 576:
                        videoresolution = '576'
                    elif videowidth <= 960 and videoheight <= 544:
                        videoresolution = '540'
                    elif videowidth <= 1280 and videoheight <= 720:
                        videoresolution = '720'
                    else:
                        videoresolution = '1080'
                else:
                    videocodec = ''
                    videoaspect = ''
                    videoresolution = ''
                listitem = xbmcgui.ListItem(label=movie, iconImage='DefaultVideo.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "genre", genre )
                listitem.setProperty( "plot", plot )
                listitem.setProperty( "plotoutline", outline )
                listitem.setProperty( "duration", runtime )
                listitem.setProperty( "studio", studio )
                listitem.setProperty( "tagline", tagline )
                listitem.setProperty( "year", year )
                listitem.setProperty( "trailer", trailer )
                listitem.setProperty( "playcount", playcount )
                listitem.setProperty( "rating", rating )
                listitem.setProperty( "starrating", starrating )
                listitem.setProperty( "mpaa", mpaa )
                listitem.setProperty( "writer", writer )
                listitem.setProperty( "director", director )
                listitem.setProperty( "videoresolution", videoresolution )
                listitem.setProperty( "videocodec", videocodec )
                listitem.setProperty( "videoaspect", videoaspect )
                listitem.setProperty( "audiocodec", audiocodec )
                listitem.setProperty( "audiochannels", audiochannels )
                listitem.setProperty( "path", path )
                listitems.append(listitem)
        self.getControl( 111 ).addItems( listitems )
        if count > 0:
            self.getControl( 110 ).setLabel( str(count) )
            self.getControl( 119 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 111 ) )
                self.focusset = 'true'

    def _fetch_tvshows( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(20343) + '[/B]' )
        count = 0
        json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "VideoLibrary.GetTVShows", "params": {"properties": ["title", "genre", "studio", "premiered", "plot", "fanart", "thumbnail", "playcount", "year", "mpaa", "episode", "rating", "art"], "sort": { "method": "label" }, "filter": {"field": "title", "operator": "contains", "value": "%s"} }, "id": 1}' % self.searchstring)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('tvshows')):
            for item in json_response['result']['tvshows']:
                tvshow = item['title']
                count = count + 1
                episode = str(item['episode'])
                fanart = item['fanart']
                genre = " / ".join(item['genre'])
                mpaa = item['mpaa']
                playcount = str(item['playcount'])
                plot = item['plot']
                premiered = item['premiered']
                rating = str(round(float(item['rating']),1))
                starrating = 'rating%.1d.png' % round(float(rating)/2)
                studio = " / ".join(item['studio'])
                thumb = item['thumbnail']
                banner = item['art'].get('banner', '')
                poster = item['art'].get('poster', '')
                tvshowid = str(item['tvshowid'])
                path = path = 'videodb://tvshows/titles/' + tvshowid + '/'
                year = str(item['year'])
                listitem = xbmcgui.ListItem(label=tvshow, iconImage='DefaultVideo.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "art(banner)", banner )
                listitem.setProperty( "art(poster)", poster )
                listitem.setProperty( "episode", episode )
                listitem.setProperty( "mpaa", mpaa )
                listitem.setProperty( "year", year )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "genre", genre )
                listitem.setProperty( "plot", plot )
                listitem.setProperty( "premiered", premiered )
                listitem.setProperty( "studio", studio )
                listitem.setProperty( "rating", rating )
                listitem.setProperty( "starrating", starrating )
                listitem.setProperty( "playcount", playcount )
                listitem.setProperty( "path", path )
                listitem.setProperty( "id", tvshowid )
                listitems.append(listitem)
        self.getControl( 121 ).addItems( listitems )
        if count > 0:
            self.getControl( 120 ).setLabel( str(count) )
            self.getControl( 129 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 121 ) )
                self.focusset = 'true'

    def _fetch_seasons( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(20343) + '[/B]' )
        count = 0
        json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "VideoLibrary.GetSeasons", "params": {"properties": ["showtitle", "season", "fanart", "thumbnail", "playcount", "episode"], "sort": { "method": "label" }, "tvshowid":%s }, "id": 1}' % self.tvshowid)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('seasons')):
            for item in json_response['result']['seasons']:
                tvshow = item['showtitle']
                count = count + 1
                episode = str(item['episode'])
                fanart = item['fanart']
                path = 'videodb://tvshows/titles/' + self.tvshowid + '/' + str(item['season']) + '/'
                season = item['label']
                playcount = str(item['playcount'])
                thumb = item['thumbnail']
                listitem = xbmcgui.ListItem(label=season, iconImage='DefaultVideo.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "episode", episode )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "tvshowtitle", tvshow )
                listitem.setProperty( "playcount", playcount )
                listitem.setProperty( "path", path )
                listitems.append(listitem)
        self.getControl( 131 ).addItems( listitems )
        if count > 0:
            self.foundseasons= 'true'
            self.getControl( 130 ).setLabel( str(count) )
            self.getControl( 139 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 131 ) )
                self.focusset = 'true'

    def _fetch_episodes( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(20360) + '[/B]' )
        count = 0
        if self.fetch_seasonepisodes == 'true':
            json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "VideoLibrary.GetEpisodes", "params": { "properties": ["title", "streamdetails", "plot", "firstaired", "runtime", "season", "episode", "showtitle", "thumbnail", "fanart", "file", "playcount", "director", "rating"], "sort": { "method": "title" }, "tvshowid":%s }, "id": 1}' % self.tvshowid)
        else:
            json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "VideoLibrary.GetEpisodes", "params": { "properties": ["title", "streamdetails", "plot", "firstaired", "runtime", "season", "episode", "showtitle", "thumbnail", "fanart", "file", "playcount", "director", "rating"], "sort": { "method": "title" }, "filter": {"field": "title", "operator": "contains", "value": "%s"} }, "id": 1}' % self.searchstring)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('episodes')):
            for item in json_response['result']['episodes']:
                if self.fetch_seasonepisodes == 'true':
                    episode = item['showtitle']
                else:
                    episode = item['title']
                count = count + 1
                if self.fetch_seasonepisodes == 'true':
                    tvshowname = episode
                    episode = item['title']
                else:
                    tvshowname = item['showtitle']
                director = " / ".join(item['director'])
                fanart = item['fanart']
                episodenumber = "%.2d" % float(item['episode'])
                path = item['file']
                plot = item['plot']
                runtime = str(int((item['runtime'] / 60.0) + 0.5))
                premiered = item['firstaired']
                rating = str(round(float(item['rating']),1))
                starrating = 'rating%.1d.png' % round(float(rating)/2)
                seasonnumber = '%.2d' % float(item['season'])
                playcount = str(item['playcount'])
                thumb = item['thumbnail']
                fanart = item['fanart']
                if item['streamdetails']['audio'] != []:
                    audiochannels = str(item['streamdetails']['audio'][0]['channels'])
                    audiocodec = str(item['streamdetails']['audio'][0]['codec'])
                else:
                    audiochannels = ''
                    audiocodec = ''
                if item['streamdetails']['video'] != []:
                    videocodec = str(item['streamdetails']['video'][0]['codec'])
                    videoaspect = float(item['streamdetails']['video'][0]['aspect'])
                    if videoaspect <= 1.4859:
                        videoaspect = '1.33'
                    elif videoaspect <= 1.7190:
                        videoaspect = '1.66'
                    elif videoaspect <= 1.8147:
                        videoaspect = '1.78'
                    elif videoaspect <= 2.0174:
                        videoaspect = '1.85'
                    elif videoaspect <= 2.2738:
                        videoaspect = '2.20'
                    else:
                        videoaspect = '2.35'
                    videowidth = item['streamdetails']['video'][0]['width']
                    videoheight = item['streamdetails']['video'][0]['height']
                    if videowidth <= 720 and videoheight <= 480:
                        videoresolution = '480'
                    elif videowidth <= 768 and videoheight <= 576:
                        videoresolution = '576'
                    elif videowidth <= 960 and videoheight <= 544:
                        videoresolution = '540'
                    elif videowidth <= 1280 and videoheight <= 720:
                        videoresolution = '720'
                    else:
                        videoresolution = '1080'
                else:
                    videocodec = ''
                    videoaspect = ''
                    videoresolution = ''
                listitem = xbmcgui.ListItem(label=episode, iconImage='DefaultVideo.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "episode", episodenumber )
                listitem.setProperty( "plot", plot )
                listitem.setProperty( "rating", rating )
                listitem.setProperty( "starrating", starrating )
                listitem.setProperty( "director", director )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "season", seasonnumber )
                listitem.setProperty( "duration", runtime )
                listitem.setProperty( "tvshowtitle", tvshowname )
                listitem.setProperty( "premiered", premiered )
                listitem.setProperty( "playcount", playcount )
                listitem.setProperty( "videoresolution", videoresolution )
                listitem.setProperty( "videocodec", videocodec )
                listitem.setProperty( "videoaspect", videoaspect )
                listitem.setProperty( "audiocodec", audiocodec )
                listitem.setProperty( "audiochannels", audiochannels )
                listitem.setProperty( "path", path )
                listitems.append(listitem)
        self.getControl( 141 ).addItems( listitems )
        if count > 0:
            self.getControl( 140 ).setLabel( str(count) )
            self.getControl( 149 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 141 ) )
                self.focusset = 'true'

    def _fetch_musicvideos( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(20389) + '[/B]' )
        count = 0
        json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "VideoLibrary.GetMusicVideos", "params": {"properties": ["title", "streamdetails", "runtime", "genre", "studio", "artist", "album", "year", "plot", "fanart", "thumbnail", "file", "playcount", "director"], "sort": { "method": "label" }, "filter": {"field": "title", "operator": "contains", "value": "%s"} }, "id": 1}' % self.searchstring)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('musicvideos')):
            for item in json_response['result']['musicvideos']:
                musicvideo = item['title']
                count = count + 1
                album = item['album']
                artist = " / ".join(item['artist'])
                director = " / ".join(item['director'])
                fanart = item['fanart']
                path = item['file']
                genre = " / ".join(item['genre'])
                plot = item['plot']
                studio = " / ".join(item['studio'])
                thumb = item['thumbnail']
                playcount = str(item['playcount'])
                year = str(item['year'])
                if year == '0':
                    year = ''
                if item['streamdetails']['audio'] != []:
                    audiochannels = str(item['streamdetails']['audio'][0]['channels'])
                    audiocodec = str(item['streamdetails']['audio'][0]['codec'])
                else:
                    audiochannels = ''
                    audiocodec = ''
                if item['streamdetails']['video'] != []:
                    videocodec = str(item['streamdetails']['video'][0]['codec'])
                    videoaspect = float(item['streamdetails']['video'][0]['aspect'])
                    if videoaspect <= 1.4859:
                        videoaspect = '1.33'
                    elif videoaspect <= 1.7190:
                        videoaspect = '1.66'
                    elif videoaspect <= 1.8147:
                        videoaspect = '1.78'
                    elif videoaspect <= 2.0174:
                        videoaspect = '1.85'
                    elif videoaspect <= 2.2738:
                        videoaspect = '2.20'
                    else:
                        videoaspect = '2.35'
                    videowidth = item['streamdetails']['video'][0]['width']
                    videoheight = item['streamdetails']['video'][0]['height']
                    if videowidth <= 720 and videoheight <= 480:
                        videoresolution = '480'
                    elif videowidth <= 768 and videoheight <= 576:
                        videoresolution = '576'
                    elif videowidth <= 960 and videoheight <= 544:
                        videoresolution = '540'
                    elif videowidth <= 1280 and videoheight <= 720:
                        videoresolution = '720'
                    else:
                        videoresolution = '1080'
                    duration = str(datetime.timedelta(seconds=int(item['streamdetails']['video'][0]['duration'])))
                    if duration[0] == '0':
                        duration = duration[2:]
                else:
                    videocodec = ''
                    videoaspect = ''
                    videoresolution = ''
                    duration = ''
                listitem = xbmcgui.ListItem(label=musicvideo, iconImage='DefaultVideo.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "album", album )
                listitem.setProperty( "artist", artist )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "director", director )
                listitem.setProperty( "genre", genre )
                listitem.setProperty( "plot", plot )
                listitem.setProperty( "duration", duration )
                listitem.setProperty( "studio", studio )
                listitem.setProperty( "year", year )
                listitem.setProperty( "playcount", playcount )
                listitem.setProperty( "videoresolution", videoresolution )
                listitem.setProperty( "videocodec", videocodec )
                listitem.setProperty( "videoaspect", videoaspect )
                listitem.setProperty( "audiocodec", audiocodec )
                listitem.setProperty( "audiochannels", audiochannels )
                listitem.setProperty( "path", path )
                listitems.append(listitem)
        self.getControl( 151 ).addItems( listitems )
        if count > 0:
            self.getControl( 150 ).setLabel( str(count) )
            self.getControl( 159 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 151 ) )
                self.focusset = 'true'

    def _fetch_artists( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(133) + '[/B]' )
        count = 0
        json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "AudioLibrary.GetArtists", "params": {"properties": ["genre", "description", "fanart", "thumbnail", "formed", "disbanded", "born", "yearsactive", "died", "mood", "style"], "sort": { "method": "label" }, "filter": {"field": "artist", "operator": "contains", "value": "%s"} }, "id": 1}' % self.searchstring)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('artists')):
            for item in json_response['result']['artists']:
                artist = item['label']
                count = count + 1
                artistid = str(item['artistid'])
                path = 'musicdb://artists/' + artistid + '/'
                born = item['born']
                description = item['description']
                died = item['died']
                disbanded = item['disbanded']
                fanart = item['fanart']
                formed = item['formed']
                genre = " / ".join(item['genre'])
                mood = " / ".join(item['mood'])
                style = " / ".join(item['style'])
                thumb = item['thumbnail']
                yearsactive = " / ".join(item['yearsactive'])
                listitem = xbmcgui.ListItem(label=artist, iconImage='DefaultArtist.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "artist_born", born )
                listitem.setProperty( "artist_died", died )
                listitem.setProperty( "artist_formed", formed )
                listitem.setProperty( "artist_disbanded", disbanded )
                listitem.setProperty( "artist_yearsactive", yearsactive )
                listitem.setProperty( "artist_mood", mood )
                listitem.setProperty( "artist_style", style )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "artist_genre", genre )
                listitem.setProperty( "artist_description", description )
                listitem.setProperty( "path", path )
                listitem.setProperty( "id", artistid )
                listitems.append(listitem)
        self.getControl( 161 ).addItems( listitems )
        if count > 0:
            self.getControl( 160 ).setLabel( str(count) )
            self.getControl( 169 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 161 ) )
                self.focusset = 'true'

    def _fetch_albums( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(132) + '[/B]' )
        count = 0
        if self.fetch_albumssongs == 'true':
            json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "AudioLibrary.GetAlbums", "params": {"properties": ["title", "description", "albumlabel", "artist", "genre", "year", "thumbnail", "fanart", "theme", "type", "mood", "style", "rating"], "sort": { "method": "label" }, "filter": {"artistid": %s} }, "id": 1}' % self.artistid)
        else:
            json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "AudioLibrary.GetAlbums", "params": {"properties": ["title", "description", "albumlabel", "artist", "genre", "year", "thumbnail", "fanart", "theme", "type", "mood", "style", "rating"], "sort": { "method": "label" }, "filter": {"field": "album", "operator": "contains", "value": "%s"} }, "id": 1}' % self.searchstring)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('albums')):
            for item in json_response['result']['albums']:
                if self.fetch_albumssongs == 'true':
                    album = " / ".join(item['artist'])
                else:
                    album = item['title']
                count = count + 1
                if self.fetch_albumssongs == 'true':
                    artist = album
                    album = item['title']
                else:
                    artist = " / ".join(item['artist'])
                    if self.fetch_songalbum == 'true':
                        if not artist == self.artistname:
                            count = count - 1
                            return
                albumid = str(item['albumid'])
                path = 'musicdb://albums/' + albumid + '/'
                label = item['albumlabel']
                description = item['description']
                fanart = item['fanart']
                genre = " / ".join(item['genre'])
                mood = " / ".join(item['mood'])
                rating = str(item['rating'])
                if rating == '48':
                    rating = ''
                if rating != '':
                    starrating = 'rating%.1d.png' % round(float(int(rating))/2)
                else:
                    starrating = 'rating0.png'
                style = " / ".join(item['style'])
                theme = " / ".join(item['theme'])
                albumtype = item['type']
                thumb = item['thumbnail']
                year = str(item['year'])
                listitem = xbmcgui.ListItem(label=album, iconImage='DefaultAlbumCover.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "artist", artist )
                listitem.setProperty( "album_label", label )
                listitem.setProperty( "genre", genre )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "album_description", description )
                listitem.setProperty( "album_theme", theme )
                listitem.setProperty( "album_style", style )
                listitem.setProperty( "album_rating", rating )
                listitem.setProperty( "starrating", starrating )
                listitem.setProperty( "album_type", albumtype )
                listitem.setProperty( "album_mood", mood )
                listitem.setProperty( "year", year )
                listitem.setProperty( "path", path )
                listitem.setProperty( "id", albumid )
                listitems.append(listitem)
        self.getControl( 171 ).addItems( listitems )
        if count > 0:
            self.getControl( 170 ).setLabel( str(count) )
            self.getControl( 179 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 171 ) )
                self.focusset = 'true'

    def _fetch_songs( self ):
        listitems = []
        self.getControl( 191 ).setLabel( '[B]' + xbmc.getLocalizedString(134) + '[/B]' )
        count = 0
        if self.fetch_albumssongs == 'true':
            json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "AudioLibrary.GetSongs", "params": {"properties": ["title", "artist", "album", "genre", "duration", "year", "file", "thumbnail", "fanart", "comment", "rating", "track", "playcount"], "sort": { "method": "title" }, "filter": {"artistid": %s} }, "id": 1}' % self.artistid)
        else:
            json_query = xbmc.executeJSONRPC('{"jsonrpc": "2.0", "method": "AudioLibrary.GetSongs", "params": {"properties": ["title", "artist", "album", "genre", "duration", "year", "file", "thumbnail", "fanart", "comment", "rating", "track", "playcount"], "sort": { "method": "title" }, "filter": {"field": "title", "operator": "contains", "value": "%s"} }, "id": 1}' % self.searchstring)
        json_query = unicode(json_query, 'utf-8', errors='ignore')
        json_response = simplejson.loads(json_query)
        if (json_response['result'] != None) and (json_response['result'].has_key('songs')):
            for item in json_response['result']['songs']:
                if self.fetch_albumssongs == 'true':
                    song = " / ".join(item['artist'])
                else:
                    song = item['title']
                count = count + 1
                if self.fetch_albumssongs == 'true':
                    artist = song
                    song = item['label']
                else:
                    artist = " / ".join(item['artist'])
                album = item['album']
                comment = item['comment']
                duration = str(datetime.timedelta(seconds=int(item['duration'])))
                if duration[0] == '0':
                    duration = duration[2:]
                fanart = item['fanart']
                path = item['file']
                genre = " / ".join(item['genre'])
                thumb = item['thumbnail']
                track = str(item['track'])
                playcount = str(item['playcount'])
                rating = str(int(item['rating'])-48)
                starrating = 'rating%.1d.png' % int(rating)
                year = str(item['year'])
                listitem = xbmcgui.ListItem(label=song, iconImage='DefaultAlbumCover.png', thumbnailImage=thumb)
                listitem.setProperty( "icon", thumb )
                listitem.setProperty( "artist", artist )
                listitem.setProperty( "album", album )
                listitem.setProperty( "genre", genre )
                listitem.setProperty( "comment", comment )
                listitem.setProperty( "track", track )
                listitem.setProperty( "rating", rating )
                listitem.setProperty( "starrating", starrating )
                listitem.setProperty( "playcount", playcount )
                listitem.setProperty( "duration", duration )
                listitem.setProperty( "fanart", fanart )
                listitem.setProperty( "year", year )
                listitem.setProperty( "path", path )
                listitems.append(listitem)
        self.getControl( 181 ).addItems( listitems )
        if count > 0:
            self.getControl( 180 ).setLabel( str(count) )
            self.getControl( 189 ).setVisible( True )
            if self.focusset == 'false':
                self.setFocus( self.getControl( 181 ) )
                self.focusset = 'true'

    def _getTvshow_Seasons( self ):
        self.fetch_seasonepisodes = 'true'
        listitem = self.getControl( 121 ).getSelectedItem()
        self.tvshowid = listitem.getProperty('id')
        self.searchstring = listitem.getLabel().replace('(','[(]').replace(')','[)]').replace('+','[+]')
        self._reset_variables()
        self._hide_controls()
        self._reset_controls()
        self._fetch_seasons()
        self._check_focus()
        self.fetch_seasonepisodes = 'false'

    def _getTvshow_Episodes( self ):
        self.fetch_seasonepisodes = 'true'
        listitem = self.getControl( 121 ).getSelectedItem()
        self.tvshowid = listitem.getProperty('id')
        self.searchstring = listitem.getLabel().replace('(','[(]').replace(')','[)]').replace('+','[+]')
        self._reset_variables()
        self._hide_controls()
        self._reset_controls()
        self._fetch_episodes()
        self._check_focus()
        self.fetch_seasonepisodes = 'false'

    def _getArtist_Albums( self ):
        self.fetch_albumssongs = 'true'
        listitem = self.getControl( 161 ).getSelectedItem()
        self.artistid = listitem.getProperty('id')
        self.searchstring = listitem.getLabel().replace('(','[(]').replace(')','[)]').replace('+','[+]')
        self._reset_variables()
        self._hide_controls()
        self._reset_controls()
        self._fetch_albums()
        self._check_focus()
        self.fetch_albumssongs = 'false'

    def _getArtist_Songs( self ):
        self.fetch_albumssongs = 'true'
        listitem = self.getControl( 161 ).getSelectedItem()
        self.artistid = listitem.getProperty('id')
        self.searchstring = listitem.getLabel().replace('(','[(]').replace(')','[)]').replace('+','[+]')
        self._reset_variables()
        self._hide_controls()
        self._reset_controls()
        self._fetch_songs()
        self._check_focus()
        self.fetch_albumssongs = 'false'

    def _getSong_Album( self ):
        self.fetch_songalbum = 'true'
        listitem = self.getControl( 181 ).getSelectedItem()
        self.artistname = listitem.getProperty('artist')
        self.searchstring = listitem.getProperty('album').replace('(','[(]').replace(')','[)]').replace('+','[+]')
        self._reset_variables()
        self._hide_controls()
        self._reset_controls()
        self._fetch_albums()
        self._check_focus()
        self.fetch_songalbum = 'false'

    def _play_video( self, path ):
        self._close()
        xbmc.Player().play( path )

    def _play_audio( self, path, listitem ):
        self._close()
        xbmc.Player().play( path, listitem )

    def _play_trailer( self ):
        self.playingtrailer = 'true'
        self.getControl( 100 ).setVisible( False )
        self.Player.play( self.trailer )

    def _trailerstopped( self ):
        self.getControl( 100 ).setVisible( True )
        self.playingtrailer = 'false'

    def _play_album( self ):
        self._close()
        xbmc.executeJSONRPC('{ "jsonrpc": "2.0", "method": "Player.Open", "params": { "item": { "albumid": %d } }, "id": 1 }' % int(self.albumid))

    def _browse_video( self, path ):
        self._close()
        xbmc.executebuiltin('ActivateWindow(Videos,' + path + ',return)')

    def _browse_audio( self, path ):
        self._close()
        xbmc.executebuiltin('ActivateWindow(MusicLibrary,' + path + ',return)')

    def _browse_album( self ):
        listitem = self.getControl( 171 ).getSelectedItem()
        path = listitem.getProperty('path')
        self._close()
        xbmc.executebuiltin('ActivateWindow(MusicLibrary,' + path + ',return)')

    def _check_focus( self ):
        self.getControl( 190 ).setLabel( '' )
        self.getControl( 191 ).setLabel( '' )
        if self.focusset == 'false':
            self.getControl( 199 ).setVisible( True )
            self.setFocus( self.getControl( 198 ) )
        self.getControl( 198 ).setVisible( True )

    def _showContextMenu( self ):
        labels = ()
        functions = ()
        controlId = self.getFocusId()
        x, y = self.getControl( controlId ).getPosition()
        w = self.getControl( controlId ).getWidth()
        h = self.getControl( controlId ).getHeight()
        if controlId == 111:
            labels += ( xbmc.getLocalizedString(13346), )
            functions += ( self._showInfo, )
            listitem = self.getControl( 111 ).getSelectedItem()
            self.trailer = listitem.getProperty('trailer')
            if self.trailer:
                labels += ( __language__(32205), )
                functions += ( self._play_trailer, )
        elif controlId == 121:
            labels += ( xbmc.getLocalizedString(20351), __language__(32207), __language__(32208), )
            functions += ( self._showInfo, self._getTvshow_Seasons, self._getTvshow_Episodes, )
        elif controlId == 131:
            labels += ( __language__(32204), )
            functions += ( self._showInfo, )
        elif controlId == 141:
            labels += ( xbmc.getLocalizedString(20352), )
            functions += ( self._showInfo, )
        elif controlId == 151:
            labels += ( xbmc.getLocalizedString(20393), )
            functions += ( self._showInfo, )
        elif controlId == 161:
            labels += ( xbmc.getLocalizedString(21891), __language__(32209), __language__(32210), )
            functions += ( self._showInfo, self._getArtist_Albums, self._getArtist_Songs, )
        elif controlId == 171:
            labels += ( xbmc.getLocalizedString(13351), __language__(32203), )
            functions += ( self._showInfo, self._browse_album, )
        elif controlId == 181:
            labels += ( xbmc.getLocalizedString(658), __language__(32206), )
            functions += ( self._showInfo, self._getSong_Album, )
        context_menu = contextmenu.GUI( "script-globalsearch-contextmenu.xml" , __cwd__, "Default", labels=labels )
        context_menu.doModal()
        if context_menu.selection is not None:
            functions[ context_menu.selection ]()
        del context_menu

    def _showInfo( self ):
        items = []
        controlId = self.getFocusId()
        if controlId == 111:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "movies"
        elif controlId == 121:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "tvshows"
        elif controlId == 131:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "seasons"
        elif controlId == 141:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "episodes"
        elif controlId == 151:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "musicvideos"
        elif controlId == 161:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "artists"
        elif controlId == 171:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "albums"
        elif controlId == 181:
            listitem = self.getControl( controlId ).getSelectedItem()
            content = "songs"
        info_dialog = infodialog.GUI( "script-globalsearch-infodialog.xml" , __cwd__, "Default", listitem=listitem, content=content )
        info_dialog.doModal()
        if info_dialog.action is not None:
            if info_dialog.action == 'play_movie':
                listitem = self.getControl( 111 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._play_video(path)
            elif info_dialog.action == 'play_trailer':
                listitem = self.getControl( 111 ).getSelectedItem()
                self.trailer = listitem.getProperty('trailer')
                self._play_trailer()
            elif info_dialog.action == 'browse_tvshow':
                listitem = self.getControl( 121 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._browse_video(path)
            elif info_dialog.action == 'browse_season':
                listitem = self.getControl( 131 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._browse_video(path)
            elif info_dialog.action == 'play_episode':
                listitem = self.getControl( 141 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._play_video(path)
            elif info_dialog.action == 'play_musicvideo':
                listitem = self.getControl( 151 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._play_video(path)
            elif info_dialog.action == 'browse_artist':
                listitem = self.getControl( 161 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._browse_audio(path)
            elif info_dialog.action == 'play_album':
                listitem = self.getControl( 171 ).getSelectedItem()
                self.albumid = listitem.getProperty('id')
                self._play_album()
            elif info_dialog.action == 'browse_album':
                listitem = self.getControl( 171 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._browse_audio(path)
            elif info_dialog.action == 'play_song':
                listitem = self.getControl( 181 ).getSelectedItem()
                path = listitem.getProperty('path')
                self._play_audio(path, listitem)
        del info_dialog

    def _newSearch( self ):
        keyboard = xbmc.Keyboard( '', __language__(32101), False )
        keyboard.doModal()
        if ( keyboard.isConfirmed() ):
            self.searchstring = keyboard.getText()
            self._reset_controls()
            self.onInit()

    def onClick( self, controlId ):
        if controlId == 111:
            listitem = self.getControl( 111 ).getSelectedItem()
            path = listitem.getProperty('path')
            self._play_video(path)
        elif controlId == 121:
            listitem = self.getControl( 121 ).getSelectedItem()
            path = listitem.getProperty('path')
            self._browse_video(path)
        elif controlId == 131:
            listitem = self.getControl( 131 ).getSelectedItem()
            path = listitem.getProperty('path')
            self._browse_video(path)
        elif controlId == 141:
            listitem = self.getControl( 141 ).getSelectedItem()
            path = listitem.getProperty('path')
            self._play_video(path)
        elif controlId == 151:
            listitem = self.getControl( 151 ).getSelectedItem()
            path = listitem.getProperty('path')
            self._play_video(path)
        elif controlId == 161:
            listitem = self.getControl( 161 ).getSelectedItem()
            path = listitem.getProperty('path')
            self._browse_audio(path)
        elif controlId == 171:
            listitem = self.getControl( 171 ).getSelectedItem()
            self.albumid = listitem.getProperty('id')
            self._play_album()
        elif controlId == 181:
            listitem = self.getControl( 181 ).getSelectedItem()
            path = listitem.getProperty('path')
            self._play_audio(path, listitem)
        elif controlId == 198:
            self._newSearch()

    def onAction( self, action ):
        if action.getId() in ACTION_CANCEL_DIALOG:
            if self.playingtrailer == 'false':
                self._close()
            else:
                self.Player.stop()
                self._trailerstopped()
        elif action.getId() in ACTION_CONTEXT_MENU:
            self._showContextMenu()
        elif action.getId() in ACTION_OSD:
            if self.playingtrailer == 'true' and xbmc.getCondVisibility('videoplayer.isfullscreen'):
                xbmc.executebuiltin("ActivateWindow(12901)")
        elif action.getId() in ACTION_SHOW_GUI:
            if self.playingtrailer == 'true':
                self.Player.stop()
                self._trailerstopped()
        elif action.getId() in ACTION_SHOW_INFO:
            if self.playingtrailer == 'true' and xbmc.getCondVisibility('videoplayer.isfullscreen'):
                xbmc.executebuiltin("ActivateWindow(142)")
            else:
                self._showInfo()

    def _close( self ):
            xbmcgui.Window(self.window_id).clearProperty('GlobalSearch.SearchString')
            log('script stopped')
            self.close()

class MyPlayer(xbmc.Player):
    def __init__(self):
        xbmc.Player.__init__( self )

    def onPlayBackEnded( self ):
        self.gui._trailerstopped()

    def onPlayBackStopped( self ):
        self.gui._trailerstopped()
