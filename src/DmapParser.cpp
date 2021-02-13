
#include "StdAfx.h"

#include "DmapParser.h"

// Code by Matt Stevens
// https://github.com/mattstevens/dmap-parser
//
//
// Copyright (c) 2011-2017 Matt Stevens
// 
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// 

typedef enum {
	DMAP_UNKNOWN = 0,
	DMAP_UINT,
	DMAP_INT,
	DMAP_STR,
	DMAP_DATA,
	DMAP_DATE,
	DMAP_VERS,
	DMAP_DICT,
	DMAP_ITEM
} DMAP_TYPE;


typedef struct dmap_field {
	/**
	 * The four-character code used in the encoded message.
	 */
	const char *code;

	/**
	 * The type of data associated with the content code.
	 */
	DMAP_TYPE type;

	/**
	 * For listings, the type of their listing item children.
	 *
	 * Listing items (mlit) can be of any type, and as with other content codes
	 * their type information is not encoded in the message. Parsers must
	 * determine the type of the listing items based on their parent context.
	 */
	DMAP_TYPE list_item_type;

	/**
	 * A human-readable name for the content code.
	 */
	const char *name;
} dmap_field;


static const dmap_field dmap_fields[] = {
	{ "abal",    DMAP_DICT, DMAP_STR,     "daap.browsealbumlisting" },
	{ "abar",    DMAP_DICT, DMAP_STR,     "daap.browseartistlisting" },
	{ "abcp",    DMAP_DICT, DMAP_STR,     "daap.browsecomposerlisting" },
	{ "abgn",    DMAP_DICT, DMAP_STR,     "daap.browsegenrelisting" },
	{ "abpl",    DMAP_UINT, DMAP_UNKNOWN, "daap.baseplaylist" },
	{ "abro",    DMAP_DICT, DMAP_UNKNOWN, "daap.databasebrowse" },
	{ "adbs",    DMAP_DICT, DMAP_UNKNOWN, "daap.databasesongs" },
	{ "aeAD",    DMAP_DICT, DMAP_UNKNOWN, "com.apple.itunes.adam-ids-array" },
	{ "aeAI",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.itms-artistid" },
	{ "aeCD",    DMAP_DATA, DMAP_UNKNOWN, "com.apple.itunes.flat-chapter-data" },
	{ "aeCF",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.cloud-flavor-id" },
	{ "aeCI",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.itms-composerid" },
	{ "aeCK",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.cloud-library-kind" },
	{ "aeCM",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.cloud-match-type" },
	{ "aeCR",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.content-rating" } ,
	{ "aeCS",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.artworkchecksum" },
	{ "aeCU",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.cloud-user-id" },
	{ "aeCd",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.cloud-id" },
	{ "aeDE",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.longest-content-description" },
	{ "aeDL",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.drm-downloader-user-id" },
	{ "aeDP",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.drm-platform-id" },
	{ "aeDR",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.drm-user-id" },
	{ "aeDV",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.drm-versions" },
	{ "aeEN",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.episode-num-str" },
	{ "aeES",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.episode-sort" },
	{ "aeFA",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.drm-family-id" },
	{ "aeGD",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.gapless-enc-dr" } ,
	{ "aeGE",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.gapless-enc-del" },
	{ "aeGH",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.gapless-heur" },
	{ "aeGI",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.itms-genreid" },
	{ "aeGR",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.gapless-resy" },
	{ "aeGU",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.gapless-dur" },
	{ "aeGs",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.can-be-genius-seed" },
	{ "aeHC",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.has-chapter-data" },
	{ "aeHD",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.is-hd-video" },
	{ "aeHV",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.has-video" },
	{ "aeK1",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.drm-key1-id" },
	{ "aeK2",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.drm-key2-id" },
	{ "aeMC",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.playlist-contains-media-type-count" },
	{ "aeMK",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.mediakind" },
	{ "aeMX",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.movie-info-xml" },
	{ "aeMk",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.extended-media-kind" },
	{ "aeND",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.non-drm-user-id" },
	{ "aeNN",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.network-name" },
	{ "aeNV",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.norm-volume" },
	{ "aePC",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.is-podcast" },
	{ "aePI",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.itms-playlistid" },
	{ "aePP",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.is-podcast-playlist" },
	{ "aePS",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.special-playlist" },
	{ "aeRD",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.rental-duration" },
	{ "aeRP",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.rental-pb-start" },
	{ "aeRS",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.rental-start" },
	{ "aeRU",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.rental-pb-duration" },
	{ "aeRf",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.is-featured" },
	{ "aeSE",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store-pers-id" },
	{ "aeSF",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.itms-storefrontid" },
	{ "aeSG",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.saved-genius" },
	{ "aeSI",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.itms-songid" },
	{ "aeSN",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.series-name" },
	{ "aeSP",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.smart-playlist" },
	{ "aeSU",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.season-num" },
	{ "aeSV",    DMAP_VERS, DMAP_UNKNOWN, "com.apple.itunes.music-sharing-version" },
	{ "aeXD",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.xid" },
	{ "aecp",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.collection-description" },
	{ "aels",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.liked-state" },
	{ "aemi",    DMAP_DICT, DMAP_UNKNOWN, "com.apple.itunes.media-kind-listing-item" },
	{ "aeml",    DMAP_DICT, DMAP_UNKNOWN, "com.apple.itunes.media-kind-listing" },
	{ "agac",    DMAP_UINT, DMAP_UNKNOWN, "daap.groupalbumcount" },
	{ "agma",    DMAP_UINT, DMAP_UNKNOWN, "daap.groupmatchedqueryalbumcount" },
	{ "agmi",    DMAP_UINT, DMAP_UNKNOWN, "daap.groupmatchedqueryitemcount" },
	{ "agrp",    DMAP_STR,  DMAP_UNKNOWN, "daap.songgrouping" },
	{ "ajAE",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.ams-episode-type" },
	{ "ajAS",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.ams-episode-sort-order" },
	{ "ajAT",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.ams-show-type" },
	{ "ajAV",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.is-ams-video" },
	{ "ajal",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.album-liked-state" },
	{ "ajcA",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.show-composer-as-artist" },
	{ "ajca",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.show-composer-as-artist" },
	{ "ajuw",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.store.use-work-name-as-display-name" },
	{ "amvc",    DMAP_UINT, DMAP_UNKNOWN, "daap.songmovementcount" },
	{ "amvm",    DMAP_STR,  DMAP_UNKNOWN, "daap.songmovementname" },
	{ "amvn",    DMAP_UINT, DMAP_UNKNOWN, "daap.songmovementnumber" },
	{ "aply",    DMAP_DICT, DMAP_UNKNOWN, "daap.databaseplaylists" },
	{ "aprm",    DMAP_UINT, DMAP_UNKNOWN, "daap.playlistrepeatmode" },
	{ "apro",    DMAP_VERS, DMAP_UNKNOWN, "daap.protocolversion" },
	{ "apsm",    DMAP_UINT, DMAP_UNKNOWN, "daap.playlistshufflemode" },
	{ "apso",    DMAP_DICT, DMAP_UNKNOWN, "daap.playlistsongs" },
	{ "arif",    DMAP_DICT, DMAP_UNKNOWN, "daap.resolveinfo" },
	{ "arsv",    DMAP_DICT, DMAP_UNKNOWN, "daap.resolve" },
	{ "asaa",    DMAP_STR,  DMAP_UNKNOWN, "daap.songalbumartist" },
	{ "asac",    DMAP_UINT, DMAP_UNKNOWN, "daap.songartworkcount" },
	{ "asai",    DMAP_UINT, DMAP_UNKNOWN, "daap.songalbumid" },
	{ "asal",    DMAP_STR,  DMAP_UNKNOWN, "daap.songalbum" },
	{ "asar",    DMAP_STR,  DMAP_UNKNOWN, "daap.songartist" },
	{ "asas",    DMAP_UINT, DMAP_UNKNOWN, "daap.songalbumuserratingstatus" },
	{ "asbk",    DMAP_UINT, DMAP_UNKNOWN, "daap.bookmarkable" },
	{ "asbo",    DMAP_UINT, DMAP_UNKNOWN, "daap.songbookmark" },
	{ "asbr",    DMAP_UINT, DMAP_UNKNOWN, "daap.songbitrate" },
	{ "asbt",    DMAP_UINT, DMAP_UNKNOWN, "daap.songbeatsperminute" },
	{ "ascd",    DMAP_UINT, DMAP_UNKNOWN, "daap.songcodectype" },
	{ "ascm",    DMAP_STR,  DMAP_UNKNOWN, "daap.songcomment" },
	{ "ascn",    DMAP_STR,  DMAP_UNKNOWN, "daap.songcontentdescription" },
	{ "asco",    DMAP_UINT, DMAP_UNKNOWN, "daap.songcompilation" },
	{ "ascp",    DMAP_STR,  DMAP_UNKNOWN, "daap.songcomposer" },
	{ "ascr",    DMAP_UINT, DMAP_UNKNOWN, "daap.songcontentrating" },
	{ "ascs",    DMAP_UINT, DMAP_UNKNOWN, "daap.songcodecsubtype" },
	{ "asct",    DMAP_STR,  DMAP_UNKNOWN, "daap.songcategory" },
	{ "asda",    DMAP_DATE, DMAP_UNKNOWN, "daap.songdateadded" },
	{ "asdb",    DMAP_UINT, DMAP_UNKNOWN, "daap.songdisabled" },
	{ "asdc",    DMAP_UINT, DMAP_UNKNOWN, "daap.songdisccount" },
	{ "asdk",    DMAP_UINT, DMAP_UNKNOWN, "daap.songdatakind" },
	{ "asdm",    DMAP_DATE, DMAP_UNKNOWN, "daap.songdatemodified" },
	{ "asdn",    DMAP_UINT, DMAP_UNKNOWN, "daap.songdiscnumber" },
	{ "asdp",    DMAP_DATE, DMAP_UNKNOWN, "daap.songdatepurchased" },
	{ "asdr",    DMAP_DATE, DMAP_UNKNOWN, "daap.songdatereleased" },
	{ "asdt",    DMAP_STR,  DMAP_UNKNOWN, "daap.songdescription" },
	{ "ased",    DMAP_UINT, DMAP_UNKNOWN, "daap.songextradata" },
	{ "aseq",    DMAP_STR,  DMAP_UNKNOWN, "daap.songeqpreset" },
	{ "ases",    DMAP_UINT, DMAP_UNKNOWN, "daap.songexcludefromshuffle" },
	{ "asfm",    DMAP_STR,  DMAP_UNKNOWN, "daap.songformat" },
	{ "asgn",    DMAP_STR,  DMAP_UNKNOWN, "daap.songgenre" },
	{ "asgp",    DMAP_UINT, DMAP_UNKNOWN, "daap.songgapless" },
	{ "asgr",    DMAP_UINT, DMAP_UNKNOWN, "daap.supportsgroups" },
	{ "ashp",    DMAP_UINT, DMAP_UNKNOWN, "daap.songhasbeenplayed" },
	{ "askd",    DMAP_DATE, DMAP_UNKNOWN, "daap.songlastskipdate" },
	{ "askp",    DMAP_UINT, DMAP_UNKNOWN, "daap.songuserskipcount" },
	{ "asky",    DMAP_STR,  DMAP_UNKNOWN, "daap.songkeywords" },
	{ "aslc",    DMAP_STR,  DMAP_UNKNOWN, "daap.songlongcontentdescription" },
	{ "aslr",    DMAP_UINT, DMAP_UNKNOWN, "daap.songalbumuserrating" },
	{ "asls",    DMAP_UINT, DMAP_UNKNOWN, "daap.songlongsize" },
	{ "aspc",    DMAP_UINT, DMAP_UNKNOWN, "daap.songuserplaycount" },
	{ "aspl",    DMAP_DATE, DMAP_UNKNOWN, "daap.songdateplayed" },
	{ "aspu",    DMAP_STR,  DMAP_UNKNOWN, "daap.songpodcasturl" },
	{ "asri",    DMAP_UINT, DMAP_UNKNOWN, "daap.songartistid" },
	{ "asrs",    DMAP_UINT, DMAP_UNKNOWN, "daap.songuserratingstatus" },
	{ "asrv",    DMAP_INT,  DMAP_UNKNOWN, "daap.songrelativevolume" },
	{ "assa",    DMAP_STR,  DMAP_UNKNOWN, "daap.sortartist" },
	{ "assc",    DMAP_STR,  DMAP_UNKNOWN, "daap.sortcomposer" },
	{ "assl",    DMAP_STR,  DMAP_UNKNOWN, "daap.sortalbumartist" },
	{ "assn",    DMAP_STR,  DMAP_UNKNOWN, "daap.sortname" },
	{ "assp",    DMAP_UINT, DMAP_UNKNOWN, "daap.songstoptime" },
	{ "assr",    DMAP_UINT, DMAP_UNKNOWN, "daap.songsamplerate" },
	{ "asss",    DMAP_STR,  DMAP_UNKNOWN, "daap.sortseriesname" },
	{ "asst",    DMAP_UINT, DMAP_UNKNOWN, "daap.songstarttime" },
	{ "assu",    DMAP_STR,  DMAP_UNKNOWN, "daap.sortalbum" },
	{ "assz",    DMAP_UINT, DMAP_UNKNOWN, "daap.songsize" },
	{ "astc",    DMAP_UINT, DMAP_UNKNOWN, "daap.songtrackcount" },
	{ "astm",    DMAP_UINT, DMAP_UNKNOWN, "daap.songtime" },
	{ "astn",    DMAP_UINT, DMAP_UNKNOWN, "daap.songtracknumber" },
	{ "asul",    DMAP_STR,  DMAP_UNKNOWN, "daap.songdataurl" },
	{ "asur",    DMAP_UINT, DMAP_UNKNOWN, "daap.songuserrating" },
	{ "asvc",    DMAP_UINT, DMAP_UNKNOWN, "daap.songprimaryvideocodec" },
	{ "asyr",    DMAP_UINT, DMAP_UNKNOWN, "daap.songyear" },
	{ "ated",    DMAP_UINT, DMAP_UNKNOWN, "daap.supportsextradata" },
	{ "avdb",    DMAP_DICT, DMAP_UNKNOWN, "daap.serverdatabases" },
	{ "awrk",    DMAP_STR,  DMAP_UNKNOWN, "daap.songwork" },
	{ "caar",    DMAP_UINT, DMAP_UNKNOWN, "dacp.availablerepeatstates" },
	{ "caas",    DMAP_UINT, DMAP_UNKNOWN, "dacp.availableshufflestates" },
	{ "caci",    DMAP_DICT, DMAP_UNKNOWN, "caci" },
	{ "cafe",    DMAP_UINT, DMAP_UNKNOWN, "dacp.fullscreenenabled" },
	{ "cafs",    DMAP_UINT, DMAP_UNKNOWN, "dacp.fullscreen" },
	{ "caia",    DMAP_UINT, DMAP_UNKNOWN, "dacp.isactive" },
	{ "cana",    DMAP_STR,  DMAP_UNKNOWN, "dacp.nowplayingartist" },
	{ "cang",    DMAP_STR,  DMAP_UNKNOWN, "dacp.nowplayinggenre" },
	{ "canl",    DMAP_STR,  DMAP_UNKNOWN, "dacp.nowplayingalbum" },
	{ "cann",    DMAP_STR,  DMAP_UNKNOWN, "dacp.nowplayingname" },
	{ "canp",    DMAP_UINT, DMAP_UNKNOWN, "dacp.nowplayingids" },
	{ "cant",    DMAP_UINT, DMAP_UNKNOWN, "dacp.nowplayingtime" },
	{ "capr",    DMAP_VERS, DMAP_UNKNOWN, "dacp.protocolversion" },
	{ "caps",    DMAP_UINT, DMAP_UNKNOWN, "dacp.playerstate" },
	{ "carp",    DMAP_UINT, DMAP_UNKNOWN, "dacp.repeatstate" },
	{ "cash",    DMAP_UINT, DMAP_UNKNOWN, "dacp.shufflestate" },
	{ "casp",    DMAP_DICT, DMAP_UNKNOWN, "dacp.speakers" },
	{ "cast",    DMAP_UINT, DMAP_UNKNOWN, "dacp.songtime" },
	{ "cavc",    DMAP_UINT, DMAP_UNKNOWN, "dacp.volumecontrollable" },
	{ "cave",    DMAP_UINT, DMAP_UNKNOWN, "dacp.visualizerenabled" },
	{ "cavs",    DMAP_UINT, DMAP_UNKNOWN, "dacp.visualizer" },
	{ "ceJC",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.jukebox-client-vote" },
	{ "ceJI",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.jukebox-current" },
	{ "ceJS",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.jukebox-score" },
	{ "ceJV",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.jukebox-vote" },
	{ "ceQR",    DMAP_DICT, DMAP_UNKNOWN, "com.apple.itunes.playqueue-contents-response" },
	{ "ceQa",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.playqueue-album" },
	{ "ceQg",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.playqueue-genre" },
	{ "ceQn",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.playqueue-name" },
	{ "ceQr",    DMAP_STR,  DMAP_UNKNOWN, "com.apple.itunes.playqueue-artist" },
	{ "cmgt",    DMAP_DICT, DMAP_UNKNOWN, "dmcp.getpropertyresponse" },
	{ "cmmk",    DMAP_UINT, DMAP_UNKNOWN, "dmcp.mediakind" },
	{ "cmpr",    DMAP_VERS, DMAP_UNKNOWN, "dmcp.protocolversion" },
	{ "cmsr",    DMAP_UINT, DMAP_UNKNOWN, "dmcp.serverrevision" },
	{ "cmst",    DMAP_DICT, DMAP_UNKNOWN, "dmcp.playstatus" },
	{ "cmvo",    DMAP_UINT, DMAP_UNKNOWN, "dmcp.volume" },
	{ "f\215ch", DMAP_UINT, DMAP_UNKNOWN, "dmap.haschildcontainers" },
	{ "ipsa",    DMAP_DICT, DMAP_UNKNOWN, "dpap.iphotoslideshowadvancedoptions" },
	{ "ipsl",    DMAP_DICT, DMAP_UNKNOWN, "dpap.iphotoslideshowoptions" },
	{ "mbcl",    DMAP_DICT, DMAP_UNKNOWN, "dmap.bag" },
	{ "mccr",    DMAP_DICT, DMAP_UNKNOWN, "dmap.contentcodesresponse" },
	{ "mcna",    DMAP_STR,  DMAP_UNKNOWN, "dmap.contentcodesname" },
	{ "mcnm",    DMAP_UINT, DMAP_UNKNOWN, "dmap.contentcodesnumber" },
	{ "mcon",    DMAP_DICT, DMAP_UNKNOWN, "dmap.container" },
	{ "mctc",    DMAP_UINT, DMAP_UNKNOWN, "dmap.containercount" },
	{ "mcti",    DMAP_UINT, DMAP_UNKNOWN, "dmap.containeritemid" },
	{ "mcty",    DMAP_UINT, DMAP_UNKNOWN, "dmap.contentcodestype" },
	{ "mdbk",    DMAP_UINT, DMAP_UNKNOWN, "dmap.databasekind" },
	{ "mdcl",    DMAP_DICT, DMAP_UNKNOWN, "dmap.dictionary" },
	{ "mdst",    DMAP_UINT, DMAP_UNKNOWN, "dmap.downloadstatus" },
	{ "meds",    DMAP_UINT, DMAP_UNKNOWN, "dmap.editcommandssupported" },
	{ "merr",    DMAP_DICT, DMAP_UNKNOWN, "dmap.error" },
	{ "meia",    DMAP_UINT, DMAP_UNKNOWN, "dmap.itemdateadded" },
	{ "meip",    DMAP_UINT, DMAP_UNKNOWN, "dmap.itemdateplayed" },
	{ "mext",    DMAP_UINT, DMAP_UNKNOWN, "dmap.objectextradata" },
	{ "miid",    DMAP_UINT, DMAP_UNKNOWN, "dmap.itemid" },
	{ "mikd",    DMAP_UINT, DMAP_UNKNOWN, "dmap.itemkind" },
	{ "mimc",    DMAP_UINT, DMAP_UNKNOWN, "dmap.itemcount" },
	{ "minm",    DMAP_STR,  DMAP_UNKNOWN, "dmap.itemname" },
	{ "mlcl",    DMAP_DICT, DMAP_DICT,    "dmap.listing" },
	{ "mlid",    DMAP_UINT, DMAP_UNKNOWN, "dmap.sessionid" },
	{ "mlit",    DMAP_ITEM, DMAP_UNKNOWN, "dmap.listingitem" },
	{ "mlog",    DMAP_DICT, DMAP_UNKNOWN, "dmap.loginresponse" },
	{ "mpco",    DMAP_UINT, DMAP_UNKNOWN, "dmap.parentcontainerid" },
	{ "mper",    DMAP_UINT, DMAP_UNKNOWN, "dmap.persistentid" },
	{ "mpro",    DMAP_VERS, DMAP_UNKNOWN, "dmap.protocolversion" },
	{ "mrco",    DMAP_UINT, DMAP_UNKNOWN, "dmap.returnedcount" },
	{ "mrpr",    DMAP_UINT, DMAP_UNKNOWN, "dmap.remotepersistentid" },
	{ "msal",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsautologout" },
	{ "msas",    DMAP_UINT, DMAP_UNKNOWN, "dmap.authenticationschemes" },
	{ "msau",    DMAP_UINT, DMAP_UNKNOWN, "dmap.authenticationmethod" },
	{ "msbr",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsbrowse" },
	{ "msdc",    DMAP_UINT, DMAP_UNKNOWN, "dmap.databasescount" },
	{ "msed",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsedit" },
	{ "msex",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsextensions" },
	{ "msix",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsindex" },
	{ "mslr",    DMAP_UINT, DMAP_UNKNOWN, "dmap.loginrequired" },
	{ "msma",    DMAP_UINT, DMAP_UNKNOWN, "dmap.machineaddress" },
	{ "msml",    DMAP_DICT, DMAP_UNKNOWN, "dmap.speakermachinelist" },
	{ "mspi",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportspersistentids" },
	{ "msqy",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsquery" },
	{ "msrs",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsresolve" },
	{ "msrv",    DMAP_DICT, DMAP_UNKNOWN, "dmap.serverinforesponse" },
	{ "mstc",    DMAP_DATE, DMAP_UNKNOWN, "dmap.utctime" },
	{ "mstm",    DMAP_UINT, DMAP_UNKNOWN, "dmap.timeoutinterval" },
	{ "msto",    DMAP_INT,  DMAP_UNKNOWN, "dmap.utcoffset" },
	{ "msts",    DMAP_STR,  DMAP_UNKNOWN, "dmap.statusstring" },
	{ "mstt",    DMAP_UINT, DMAP_UNKNOWN, "dmap.status" },
	{ "msup",    DMAP_UINT, DMAP_UNKNOWN, "dmap.supportsupdate" },
	{ "mtco",    DMAP_UINT, DMAP_UNKNOWN, "dmap.specifiedtotalcount" },
	{ "mudl",    DMAP_DICT, DMAP_UNKNOWN, "dmap.deletedidlisting" },
	{ "mupd",    DMAP_DICT, DMAP_UNKNOWN, "dmap.updateresponse" },
	{ "musr",    DMAP_UINT, DMAP_UNKNOWN, "dmap.serverrevision" },
	{ "muty",    DMAP_UINT, DMAP_UNKNOWN, "dmap.updatetype" },
	{ "pasp",    DMAP_STR,  DMAP_UNKNOWN, "dpap.aspectratio" },
	{ "pcmt",    DMAP_STR,  DMAP_UNKNOWN, "dpap.imagecomments" },
	{ "peak",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.photos.album-kind" },
	{ "peed",    DMAP_DATE, DMAP_UNKNOWN, "com.apple.itunes.photos.exposure-date" },
	{ "pefc",    DMAP_DICT, DMAP_UNKNOWN, "com.apple.itunes.photos.faces" },
	{ "peki",    DMAP_UINT, DMAP_UNKNOWN, "com.apple.itunes.photos.key-image-id" },
	{ "pekm",    DMAP_DICT, DMAP_UNKNOWN, "com.apple.itunes.photos.key-image" },
	{ "pemd",    DMAP_DATE, DMAP_UNKNOWN, "com.apple.itunes.photos.modification-date" },
	{ "pfai",    DMAP_DICT, DMAP_UNKNOWN, "dpap.failureids" },
	{ "pfdt",    DMAP_DICT, DMAP_UNKNOWN, "dpap.filedata" },
	{ "pfmt",    DMAP_STR,  DMAP_UNKNOWN, "dpap.imageformat" },
	{ "phgt",    DMAP_UINT, DMAP_UNKNOWN, "dpap.imagepixelheight" },
	{ "picd",    DMAP_DATE, DMAP_UNKNOWN, "dpap.creationdate" },
	{ "pifs",    DMAP_UINT, DMAP_UNKNOWN, "dpap.imagefilesize" },
	{ "pimf",    DMAP_STR,  DMAP_UNKNOWN, "dpap.imagefilename" },
	{ "plsz",    DMAP_UINT, DMAP_UNKNOWN, "dpap.imagelargefilesize" },
	{ "ppro",    DMAP_VERS, DMAP_UNKNOWN, "dpap.protocolversion" },
	{ "prat",    DMAP_UINT, DMAP_UNKNOWN, "dpap.imagerating" },
	{ "pret",    DMAP_DICT, DMAP_UNKNOWN, "dpap.retryids" },
	{ "pwth",    DMAP_UINT, DMAP_UNKNOWN, "dpap.imagepixelwidth" }
};
static const size_t dmap_field_count = sizeof(dmap_fields) / sizeof(dmap_field);


typedef int (*sort_func) (const void *, const void *);


static int dmap_type_sort(const dmap_field *a, const dmap_field *b) {
	return strncmp(a->code, b->code, 4);
}


static const dmap_field *dmap_field_from_code(const char *code) {
	dmap_field key;
	key.code = code;
	return (const dmap_field *)bsearch(&key, dmap_fields, dmap_field_count, sizeof(dmap_field), (sort_func)dmap_type_sort);
}


const char *CDmapParser::dmap_name_from_code(const char *code) {
	const dmap_field *t = dmap_field_from_code(code);
	return t != 0 ? t->name : 0;
}


static int16_t dmap_read_i16(const char *buf) {
	return (int16_t)((buf[0] & 0xff) <<  8) |
	((buf[1] & 0xff));
}


static uint16_t dmap_read_u16(const char *buf) {
	return (uint16_t)((buf[0] & 0xff) <<  8) |
	((buf[1] & 0xff));
}


static int32_t dmap_read_i32(const char *buf) {
	return ((int32_t)(buf[0] & 0xff) << 24) |
	((int32_t)(buf[1] & 0xff) << 16) |
	((int32_t)(buf[2] & 0xff) <<  8) |
	((int32_t)(buf[3] & 0xff));
}


static uint32_t dmap_read_u32(const char *buf) {
	return ((uint32_t)(buf[0] & 0xff) << 24) |
	((uint32_t)(buf[1] & 0xff) << 16) |
	((uint32_t)(buf[2] & 0xff) <<  8) |
	((uint32_t)(buf[3] & 0xff));
}


static int64_t dmap_read_i64(const char *buf) {
	return ((int64_t)(buf[0] & 0xff) << 56) |
	((int64_t)(buf[1] & 0xff) << 48) |
	((int64_t)(buf[2] & 0xff) << 40) |
	((int64_t)(buf[3] & 0xff) << 32) |
	((int64_t)(buf[4] & 0xff) << 24) |
	((int64_t)(buf[5] & 0xff) << 16) |
	((int64_t)(buf[6] & 0xff) <<  8) |
	((int64_t)(buf[7] & 0xff));
}


static uint64_t dmap_read_u64(const char *buf) {
	return ((uint64_t)(buf[0] & 0xff) << 56) |
	((uint64_t)(buf[1] & 0xff) << 48) |
	((uint64_t)(buf[2] & 0xff) << 40) |
	((uint64_t)(buf[3] & 0xff) << 32) |
	((uint64_t)(buf[4] & 0xff) << 24) |
	((uint64_t)(buf[5] & 0xff) << 16) |
	((uint64_t)(buf[6] & 0xff) <<  8) |
	((uint64_t)(buf[7] & 0xff));
}

/////////////////////////////////////////////////////////////////////////
// CDmapParser

CDmapParser::CDmapParser(void)
{
}


CDmapParser::~CDmapParser(void)
{
}

int CDmapParser::dmap_parse_internal(void* ctx, const char *buf, size_t len, const struct dmap_field *parent)
{
	const dmap_field *field;
	DMAP_TYPE field_type;
	size_t field_len;
	const char *field_name;
	const char *p = buf;
	const char *end = buf + len;
	char code[5] = {0};
	code[4] = '\0';

	while (end - p >= 8) {
		strncpy_s(code, sizeof(code), p, 4);
		field = dmap_field_from_code(code);
		p += 4;

		field_len = dmap_read_u32(p);
		p += 4;

		if (p + field_len > end)
			return -1;

		if (field) {
			field_type = field->type;
			field_name = field->name;

			if (field_type == DMAP_ITEM) {
				if (parent != NULL && parent->list_item_type) {
					field_type = parent->list_item_type;
				} else {
					field_type = DMAP_DICT;
				}
			}
		} else {
			/* Make a best guess of the type */
			field_type = DMAP_UNKNOWN;
			field_name = code;

			if (field_len >= 8) {
				/* Look for a four char code followed by a length within the current field */
				if (isalpha(p[0] & 0xff) &&
				    isalpha(p[1] & 0xff) &&
				    isalpha(p[2] & 0xff) &&
				    isalpha(p[3] & 0xff)) {
					if (dmap_read_u32(p + 4) < field_len)
						field_type = DMAP_DICT;
				}
			}

			if (field_type == DMAP_UNKNOWN) {
				size_t i;
				int is_string = 1;
				for (i=0; i < field_len; i++) {
					if (!isprint(p[i] & 0xff)) {
						is_string = 0;
						break;
					}
				}

				field_type = is_string ? DMAP_STR : DMAP_UINT;
			}
		}

		switch (field_type) {
			case DMAP_UINT:
				/* Determine the integer's type based on its size */
				switch (field_len) {
					case 1:
						on_uint32(ctx, code, field_name, *p);
						break;
					case 2:
						on_uint32(ctx, code, field_name, dmap_read_u16(p));
						break;
					case 4:
						on_uint32(ctx, code, field_name, dmap_read_u32(p));
						break;
					case 8:
						on_uint64(ctx, code, field_name, dmap_read_u64(p));
						break;
					default:
						on_data(ctx, code, field_name, p, field_len);
						break;
				}
				break;
			case DMAP_INT:
				switch (field_len) {
					case 1:
						on_int32(ctx, code, field_name, *p);
						break;
					case 2:
						on_int32(ctx, code, field_name, dmap_read_i16(p));
						break;
					case 4:
						on_int32(ctx, code, field_name, dmap_read_i32(p));
						break;
					case 8:
						on_int64(ctx, code, field_name, dmap_read_i64(p));
						break;
					default:
						on_data(ctx, code, field_name, p, field_len);
						break;
				}
				break;
			case DMAP_STR:
				on_string(ctx, code, field_name, p, field_len);
				break;
			case DMAP_DATA:
				on_data(ctx, code, field_name, p, field_len);
				break;
			case DMAP_DATE:
				/* Seconds since epoch */
				on_date(ctx, code, field_name, dmap_read_u32(p));
				break;
			case DMAP_VERS:
				if (field_len >= 4) {
					char version[20];
					sprintf_s(version, 20, "%u.%u", dmap_read_u16(p), dmap_read_u16(p+2));
					on_string(ctx, code, field_name, version, strlen(version));
				}
				break;
			case DMAP_DICT:
				on_dict_start(ctx, code, field_name);
				if (dmap_parse_internal(ctx, p, field_len, field) != 0)
					return -1;
				on_dict_end(ctx, code, field_name);
				break;
			case DMAP_ITEM:
				/* Unreachable: listing item types are always mapped to another type */
				abort();
			case DMAP_UNKNOWN:
				break;
		}

		p += field_len;
	}

	if (p != end)
		return -1;

	return 0;
}

int CDmapParser::dmap_parse(void* ctx, const char *buf, size_t len)
{
	return dmap_parse_internal(ctx, buf, len, NULL);
}
