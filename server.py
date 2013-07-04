# -*- coding: utf-8 -*-

#    This file is part of Mumoro.
#
#    Mumoro is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Mumoro is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Mumoro.  If not, see <http://www.gnu.org/licenses/>.
#
#    © Université de Toulouse 1 2010
#    Author: Tristram Gräbener, Odysseas Gabrielides


from lib.core import mumoro
from lib.core.mumoro import Bike, Car, Foot, PublicTransport
from lib import layer
from Builder import *

from lib import bikestations as bikestations
from web import shorturl

from sqlalchemy import *
from sqlalchemy.orm import mapper, sessionmaker, clear_mappers

import cherrypy
import sys
import simplejson as json
import os
import time
import urllib
import httplib
import hashlib
import datetime

from cherrypy import request
from genshi.template import TemplateLoader

loader = TemplateLoader(
    os.path.join(os.path.dirname(__file__), 'web/templates'),
    auto_reload=True
)


class Mumoro:
    def __init__(self,db_string,config_file,admin_email,web_url, graph):
        self.g = graph
        if not admin_email:
            raise NameError('Administrator email is empty')
        self.admin_email = admin_email
        if not web_url:
            raise NameError('Website URL is empty')
        self.web_url = web_url
        if not layer_array:
                raise NameError('Can not create multimodal graph beceause there are no layers')
        layers = []
        for i in layer_array:
            layers.append( i['layer'] )
        for i in range( len( layer_array ) ):
            for j in range( len( layer_array ) ):
                if i != j:
                    if layer_array[i]['name'] == layer_array[j]['name']:
                        raise NameError('Layers can not have the same name')
        if len( paths_array ) == 0:
            print 'Warning: there are no defined paths !'
        self.engine = create_engine(db_string)
        self.metadata = MetaData(bind = self.engine)
        Session = sessionmaker(bind=self.engine)
        self.session = Session()
        self.bike_stations = []
        for i in bike_stations_array:
            self.bike_stations.append( bikestations.BikeStationImporter(i['url_api'],i['table'],self.metadata) )
        for i in self.bike_stations:
            i.update_from_db()
        self.timestamp = time.time()
        self.config_table = Table('config', self.metadata,
            Column('config_file', String, primary_key = True),
            Column('binary_file', String, primary_key = True),
            Column('md5', String, index = True)
            )
        self.hash_table = Table('hurl', self.metadata,
        Column('id', String(length=16), primary_key=True),
        Column('zoom', Integer),
        Column('lonMap', Float),
        Column('latMap', Float),
        Column('lonStart', Float),
        Column('latStart', Float),
        Column('lonDest', Float),
        Column('latDest', Float),
        Column('addressStart', Text),
        Column('addressDest', Text),
        Column('chrone', DateTime(timezone=False))
        )
        self.metadata.create_all()
        s = self.config_table.select()
        rs = s.execute()
        row = rs.fetchone()
          

    @cherrypy.expose
    def path(self, slon, slat, dlon, dlat, time):
        start = self.g.match( 'Street', float(slon), float(slat))
        dest = self.g.match( 'Street', float(dlon), float(dlat))
        date = self.analyse_date( time )
        print "Searching path from {0} to {1} at time {2} on day {3}".format(start, dest, date['seconds'], date['days'])
        #res = mumoro.cap_jj_nf( self.g.graph() ).visualization()
        #iso = mumoro.isochrone( self.g.graph(), mumoro.foot_subway_dfa(), start, 1200 )
        #res = iso.visualization()
        
        #res = mumoro.rectangle_containing( self.g.graph(), start, dest, 0.015 ).visualization()
        
        res = self.muparo( start, dest, date['seconds'], date['days'], self.g.graph() )
        return self.pathToGeoJson( res )
    
    def regular_dij_path(self, start, dest, secs, day, graph ):
        return mumoro.dijkstra( start, dest, secs, day, graph )
    
    def reglc_dij_path(self, start, dest, secs, day, graph ):
        rlc = mumoro.RLC_Graph(graph, mumoro.pt_foot_dfa())
        dij = mumoro.Dijkstra(rlc, start, dest, secs, day)
        dij.run()
        return dij.get_result()
    
    def back_dij_path(self, start, dest, secs, day, graph ):
        rlc = mumoro.RLC_Graph(graph, mumoro.car_dfa())
        back_rlc = mumoro.BackwardGraph(rlc)
        dij = mumoro.Dijkstra(back_rlc, start, dest, secs, day)
        dij.run()
        return dij.get_result()
    
    def muparo(self, start, dest, secs, day, graph ):
        res = mumoro.point_to_point(graph, start, dest, secs, mumoro.car_dfa() )
        #res = mumoro.show_shared_path( graph, start, dest, 306 )
        #res = mumoro.show_car_sharing(graph, start, dest, 473972, 555680, mumoro.pt_foot_dfa(), mumoro.car_dfa())
        #print "{} {} {} {} {} {} {} {}".format(start, dest, 278790, 112254, secs, 10, 1, 0)
        #g = mumoro.RLC_Graph( graph, mumoro.pt_foot_dfa() )
        #res = mumoro.show_isochrone( g, dest, 120 )
        
        #res = mumoro.show_car_sharing(graph,112748, 104371, 603225, 397968 , mumoro.pt_foot_dfa(), mumoro.car_dfa())
        
        #res = mumoro.show_meeting_points( graph, start )
        
        #mumoro.free(mpr)
        return res
    
    @cherrypy.expose
    def bikes(self):
        if len( self.bike_stations ) > 0:
            if time.time() > self.timestamp + 60 * 5:
                print "Updating bikestations"
                for i in self.bike_stations:
                    i.import_data()
                print "Done !"
            for i in self.bike_stations:
                i.update_from_db()
            res = 'lat\tlon\ttitle\tdescription\ticon\ticonSize\ticonOffset\n'
            for i in self.bike_stations:
                res += i.to_string()
            print "Got string"
            return res;
        else:
            print "No bike stations imported so no string available to generate"
            return None

    @cherrypy.expose
    def addhash(self,mlon,mlat,zoom,slon,slat,dlon,dlat,saddress,daddress,time):
        cherrypy.response.headers['Content-Type']= 'application/json'
        hashAdd = shorturl.shortURL(self.metadata)
        hmd5 =hashAdd.addRouteToDatabase(mlon,mlat,zoom,slon,slat,dlon,dlat,saddress,daddress, time)
        if( len(hmd5) > 0 ):
            ret = {
                'h': hmd5
            }
            return json.dumps(ret)
        else:
            return '{"error": "Add to DB failed"}'

    @cherrypy.expose
    def h(self,id):
        hashCheck = shorturl.shortURL(self.metadata)
        res = hashCheck.getDataFromHash(id)
        if( len(res) > 0 ):
            return self.index(True,res)
        else:
            return self.index(False,res)

    @cherrypy.expose
    def index(self,fromHash=False,hashData=[]):
        tmpl = loader.load('index.html')
        t = "{"
        for i in range( len( layer_array ) ):
            t = t + "\"" + layer_array[i]['name'] + "\": { strokeColor : \"" + layer_array[i]['color'] + "\"}"
            if i != len( layer_array ) - 1 :
                t = t + ","
        t = t + "}"
        if( not fromHash ):
            a = paths_array[0]['starting_layer']['layer'].average()
            b = paths_array[0]['starting_layer']['layer'].borders()
            return tmpl.generate(fromHash='false',lonMap=a['avg_lon'],latMap=a['avg_lat'],zoom=14,lonStart=b['min_lon'],latStart=b['min_lat'],lonDest=b['max_lon'],latDest=b['max_lat'],addressStart='',addressDest='',hashUrl=self.web_url,layers=t, date=datetime.datetime.today().strftime("%d/%m/%Y %H:%M")).render('html', doctype='html')
        else:
            return tmpl.generate(fromHash='true',lonMap=hashData[2],latMap=hashData[3],zoom=hashData[1],lonStart=hashData[4],latStart=hashData[5],lonDest=hashData[6],latDest=hashData[7],addressStart=hashData[8].decode('utf-8'),addressDest=hashData[9].decode('utf-8'),hashUrl=self.web_url,layers=t,date=hashData[10]).render('html', doctype='html')
    index.exposed = True

    @cherrypy.expose
    def info(self):
        tmpl = loader.load('info.html')
        return tmpl.generate().render('html', doctype='html')
    
    @cherrypy.expose
    def geo(self,q):
        cherrypy.response.headers['Content-Type']= 'application/json'
        url = "nominatim.openstreetmap.org:80"
        params = urllib.urlencode({
          "q": q.encode("utf-8"),
          "format":"json",
          "polygon": 0,
          "addressdetails" : 1,
          "email" : self.admin_email
        })
        conn = httplib.HTTPConnection(url)
        conn.request("GET", "/search?" + params)
        response = conn.getresponse()
        ret = json.loads(response.read())
        is_covered = False
        if ret:
                cord_error = ""
                lon = ret[0]['lon']
                lat = ret[0]['lat']
                display_name = ret[0]['display_name']
                if self.arecovered(lon,lat):
                        node_error = ""
                        is_covered = True
                else:
                        node_error = "not covered area"
        else:
                cord_error = "geocoding failed"
                lon = 0
                lat = 0
                display_name = ""
                node_error = "match failed because geocoding failed"
        data = {
                'lon': lon,
                'lat': lat,
                'display_name': display_name,
                'node_error': node_error,
                'cord_error': cord_error,
                'is_covered': is_covered
        }
        return json.dumps(data)

    @cherrypy.expose
    def revgeo(self,lon,lat):
        cherrypy.response.headers['Content-Type']= 'application/json'
        url = "nominatim.openstreetmap.org:80"
        params = urllib.urlencode({
          "lon": lon,
          "lat": lat,
          "format":"json",
          "zoom": 18,
          "addressdetails" : 1,
          "email" : self.admin_email
        })
        conn = httplib.HTTPConnection(url)
        conn.request("GET", "/reverse?" + params)
        response = conn.getresponse()
        ret = json.loads(response.read())
        is_covered = False
        if ret:
                cord_error = ""
                display_name = ret['display_name']
                if self.arecovered(float(lon),float(lat)):
                        node_error = ""
                        is_covered = True
                else:
                        node_error = "not covered area"
        else:
                cord_error = "geocoding failed"
                display_name = ""
                node_error = "match failed because revgeocoding failed"
        data = {
                'display_name': display_name,
                'node_error': node_error,
                'cord_error': cord_error,
                'is_covered': is_covered
        }
        return json.dumps(data)

    def arecovered(self,lon,lat):
        return True
        for i in layer_array:
            b = i['layer'].borders()
            if lon >= b['min_lon'] and lon <= b['max_lon'] and lat >= b['min_lat'] and lat <= b['max_lat']:
                return True
            else:
                print lon,lat, b
        return False

    def normalise_paths(self,route,used_objectives,union_objectives):
        for i in route:
            if len( union_objectives ) + 1 != len( i.cost ):
                tmp = Costs()
                tmp.append( i.cost[0] )
                for j in range( len( union_objectives ) ):
                    if j < len( used_objectives ):
                        if used_objectives[j] == union_objectives[j]:
                            tmp.append( i.cost[j + 1] )
                        else:
                            tmp.append( 0.0 )
                    else:
                         tmp.append( 0.0 )
                i.cost = tmp
        return route

    def analyse_date(self,date):
        #now_chrone = datetime.datetime.strptime(date, "%Y-%m-%d %H:%M:%S")
        now_chrone = datetime.datetime.strptime(date, "%d/%m/%Y %H:%M")
        start_chrone = datetime.datetime.strptime(start_date, "%Y%m%d")
        past_seconds = now_chrone.hour * 60 * 60 + now_chrone.minute * 60 + now_chrone.second
        delta = now_chrone - start_chrone
        return {'seconds':past_seconds,'days':delta.days} 
    
    @cherrypy.expose
    def edgeFeatures(self, restriction):
        edges = None
        if not restriction or restriction == '':
            edges = self.g.graph().listEdges()
        elif restriction == 'Bus':
            edges = self.g.graph().listEdges(mumoro.BusEdge)
        elif restriction == 'Bike':
            edges = self.g.graph().listEdges(mumoro.BikeEdge)
        elif restriction == 'Foot':
            edges = self.g.graph().listEdges(mumoro.FootEdge)
        elif restriction == 'Car':
            edges = self.g.graph().listEdges(mumoro.CarEdge)
        elif restriction == 'Subway':
            edges = self.g.graph().listEdges(mumoro.SubwayEdge)
        elif restriction == 'Transfer':
            edges = self.g.graph().listEdges(mumoro.TransferEdge)
        elif restriction == 'Tram':
            edges = self.g.graph().listEdges(mumoro.TramEdge)
        return self.edgesToFeatures(edges)

    def pathToGeoJson(self, res):
        ret = {
                'objectives': '',
                'paths': []
                }
        p_str = {
                'cost': [],
                'type': 'FeatureCollection',
                'crs': {
                    'type': 'EPSG',
                    'properties': {
                        'code': 4326,
                        'coordinate_order': [1,0]
                        }
                    }
                }
        features = []
        feature = {'type': 'feature'}
        geometry = {'type': 'Linestring'}
        coordinates = []
        for edge_id in res.edges:
            src_coord = self.g.coordinates(self.g.graph().source(edge_id))
            target_coord = self.g.coordinates(self.g.graph().target(edge_id))
            feature = {'type': 'feature'}
            geometry = {'type': 'Linestring'}
            coordinates = []

            connection = {
                    'type': 'feature',
                    'geometry': {
                        'type': 'Linestring',
                        'coordinates': [[src_coord[0], src_coord[1]], [target_coord[0], target_coord[1]]]
                        },
                    'properties': { 'layer': EdgeTypesToString[self.g.graph().map(edge_id).type] }
                    }
            features.append(connection);
        
        #for (nodes, layer) in ((res.a_nodes, 'PointA'), (res.b_nodes, 'PointB'), (res.c_nodes, 'PointC')):
        for node_id in [ res.start_node, res.end_node ]:
            node = self.g.graph().mapNode(node_id)
            feature = {
                'type': 'feature',
                'geometry': {
                    'type': 'Point',
                    'coordinates': [node.lon, node.lat]
                    },
                'properties': { 'layer': 'PointA' }
                }
            features.append(feature)
                    

        p_str['features'] = features
        ret['paths'].append(p_str)
        print ret
        return json.dumps(ret)

EdgeTypesToString = [ 'Foot', 'Bike', 'Car', 'Subway', 'Bus', 'Tram', 'Transfer', 'Unknown', 'All' ]
StringToEdgeType = { 'Foot': mumoro.FootEdge, 'Bike': mumoro.BikeEdge, 'Car': mumoro.CarEdge, 
                     'Subway': mumoro.SubwayEdge, 'Bus': mumoro.BusEdge, 'Tram': mumoro.TramEdge, 
                     'Transfer': mumoro.TransferEdge, 'Unknown': mumoro.UnknownEdgeType, 'All': mumoro.WhateverEdge }




exec( file('Builder.py') )

if __name__ == "__main__":
    total = len( sys.argv )
    if total != 2:
        sys.exit("Usage: python data_import.py {data_directory}")
        
    if not os.path.exists( os.getcwd() + "/" + sys.argv[1] ):
        raise NameError('Data directory does not exist')
    
    data_dir = os.getcwd() + "/" + sys.argv[1]
    
    if not os.path.exists( data_dir + "conf.py" ):
        raise NameError('Data directory does not contain a conf.py file')
    
    exec( file( data_dir + 'conf.py' ) )
    db_string = db_type + ":///" + db_params

    builder = Builder( os.getcwd() + "/" + sys.argv[1], source='bin-dump' ) 

    cherrypy.config.update({
        'tools.encode.on': True,
        'tools.encode.encoding': 'utf-8',
        'tools.decode.on': True,
        'tools.trailing_slash.on': True,
        'tools.staticdir.root': os.path.abspath(os.path.dirname(__file__)) + "/web/",
        'server.socket_port': listening_port,
        'server.socket_host': '0.0.0.0'
    })

    cherrypy.quickstart( Mumoro(db_type + ":///" + db_params,sys.argv[1],admin_email,web_url, builder.g ), '/', config={
        '/': {
                'tools.staticdir.on': True,
                'tools.staticdir.dir': 'static'
        },
    })

def main():
    print "Goodbye!"
    

