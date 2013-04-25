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

layer_array = []
bike_stations_array = []
same_nodes_connection_array = []
nearest_nodes_connection_array = []
nodes_list_connection_array = []

paths_array = []

def md5_of_file(filename):
    block_size=2**20
    md5 = hashlib.md5()
    while True:
        data = filename.read(block_size)
        if not data:
            break
        md5.update(data)
    filename.close()
    return md5.hexdigest()

def is_color_valid( color ):
    if len( color ) == 7:
        if color[0] == '#':
            try:
                r = int( color[1:3], 16)
                if r <= 255 and r >= 0:
                    try:
                        g = int( color[3:5], 16)
                        if g <= 255 and g >= 0:
                            try:
                                b = int( color[5:7], 16)
                                if b <= 255 and b >= 0:
                                    return True
                            except ValueError:
                                return False
                    except ValueError:
                        return False
            except ValueError:
                return False
    return False

#Loads an osm (compressed of not) file and insert data into database
def import_street_data( filenames ):
    origin = str( filenames )
    engine = create_engine( db_type + ":///" + db_params )
    metadata = MetaData(bind = engine)
    mumoro_metadata = Table('metadata', metadata, autoload = True)
    s = mumoro_metadata.select((mumoro_metadata.c.origin == origin) & (mumoro_metadata.c.node_or_edge == 'Nodes'))
    rs = s.execute()
    nd = 0
    for row in rs:
         nd = row[0]
    s = mumoro_metadata.select((mumoro_metadata.c.origin == origin) & (mumoro_metadata.c.node_or_edge == 'Edges'))
    rs = s.execute()
    ed = 0
    for row in rs:
         ed = row[0]
    return {'nodes': str(nd), 'edges' : str(ed)}


# Loads the tables corresponding to the public transport layer
def import_gtfs_data( filename, network_name = "Public Transport"):
    engine = create_engine(db_type + ":///" + db_params)
    metadata = MetaData(bind = engine)
    mumoro_metadata = Table('metadata', metadata, autoload = True)
    nd = mumoro_metadata.select((mumoro_metadata.c.origin == filename) & (mumoro_metadata.c.node_or_edge == 'Nodes')).execute().first()[0]

    ed = mumoro_metadata.select((mumoro_metadata.c.origin == filename) & (mumoro_metadata.c.node_or_edge == 'Edges')).execute().first()[0]
    
    services = mumoro_metadata.select((mumoro_metadata.c.origin == filename) & (mumoro_metadata.c.node_or_edge == 'Services')).execute().first()[0]

    return {'nodes': str(nd), 'edges' : str(ed), 'services': str(services) }

def import_kalkati_data(filename, network_name = "Public Transport"):
    return import_gtfs_data(filename, network_name)

def import_freq(self, line_name, nodesf, linesf):
    return import_gtfs_data(line_name)

#Loads a bike service API ( from already formatted URL ). Insert bike stations in database and enables schedulded re-check.
def import_bike_service( url, name ):
    engine = create_engine(db_type + ":///" + db_params)
    metadata = MetaData(bind = engine)
    mumoro_metadata = Table('metadata', metadata, autoload = True)
    s = mumoro_metadata.select((mumoro_metadata.c.origin == url) & (mumoro_metadata.c.node_or_edge == 'bike_stations'))
    rs = s.execute()
    for row in rs:
         bt = row[0]
    bike_stations_array.append( {'url_api': url,'table': str(bt)} )
    return {'url_api': url,'table': str(bt)}

#Loads data from previous inserted data and creates a layer used in multi-modal graph
def street_layer( data, name, color, mode ):
    if not data or not name:
        raise NameError('One or more parameters are missing')
    if not is_color_valid( color ):
        raise NameError('Color for the layer is invalid')
    if mode != mumoro.Foot and mode != mumoro.Bike and mode != mumoro.Car and mode != None:
        raise NameError('Wrong layer mode paramater')
    engine = create_engine(db_type + ":///" + db_params)
    metadata = MetaData(bind = engine)
    res = layer.Layer(name, mode, data, metadata)
    layer_array.append( {'layer':res,'name':name,'mode':mode,'origin':data,'color':color} )
    return {'layer':res,'name':name,'mode':mode,'origin':data,'color':color}

def mixed_street_layer( data, name, color ):
    if not data or not name:
        raise NameError('One or more parameters are missing')
    if not is_color_valid( color ):
        raise NameError('Color for the layer is invalid')
    engine = create_engine(db_type + ":///" + db_params)
    metadata = MetaData(bind = engine)
    res = layer.MixedStreetLayer(name, data, metadata)
    layer_array.append( {'layer':res,'name':name,'origin':data,'color':color} )
    return {'layer':res,'name':name,'origin':data,'color':color}
    
    
def public_transport_layer(data, name, color): 
    engine = create_engine(db_type + ":///" + db_params)
    metadata = MetaData(bind = engine)
    res = layer.GTFSLayer(name, data, metadata)
    layer_array.append( {'layer':res,'name':name,'mode':PublicTransport,'origin':data,'color':color} )
    return {'layer':res,'name':name,'mode':PublicTransport,'origin':PublicTransport,'color':color} 

def paths( starting_layer, destination_layer, objectives ):
    if not starting_layer or not destination_layer:
        raise NameError('Empty layer(s)')
    for i in range( len( objectives ) ):
        if objectives[i] != mumoro.dist and objectives[i] != mumoro.cost and objectives[i] != mumoro.elevation and objectives[i] != mumoro.co2 and objectives[i] != mumoro.mode_change and objectives[i] != mumoro.line_change:
            raise NameError('Wrong objective parameter')
    paths_array.append( {'starting_layer':starting_layer,'destination_layer':destination_layer,'objectives':objectives} )

#Creates a transit cost variable, including the duration in seconds of the transit and if the mode is changed
def transferEdge( duration, mode_change ):
    return { 'duration': duration, 'type': mumoro.TransferEdge }

#Connects 2 given layers on same nodes with the given cost(s)
def connect_layers_same_nodes( layer1, layer2, cost ):
    if not layer1 or not layer2:
        raise NameError('One or more paramaters are empty')
    same_nodes_connection_array.append( { 'layer1':layer1, 'layer2':layer2, 'property':transferEdge(cost, 0) } )

#Connect 2 given layers on a node list (arg 3 which should be the returned data from import_municipal_data or import_bike_service) with the given cost(s)
def connect_layers_from_node_list( layer1, layer2, node_list, cost, cost2 = None ):
    if not layer1 or not layer2 or not node_list or not cost:
        raise NameError('One or more paramaters are empty')
    if not cost2:
        nodes_list_connection_array.append( { 'layer1':layer1, 'layer2':layer2, 'node_list':node_list, 'cost1':cost, 'cost2':cost } )
    else:
        nodes_list_connection_array.append( { 'layer1':layer1, 'layer2':layer2, 'node_list':node_list, 'cost1':cost, 'cost2':cost2 } )

#Connect 2 given layers on nearest nodes
def connect_layers_on_nearest_nodes( layer1 , layer2, cost, cost2 = None):
    if not layer1 or not layer2 or not cost:
        raise NameError('One or more paramaters are empty')
    nearest_nodes_connection_array.append( { 'layer1':layer1, 'layer2':layer2, 'cost':cost, 'cost2':cost2 } )

class Mumoro:
    def __init__(self,db_string,config_file,admin_email,web_url):
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
        if row and row['md5']== md5_of_file( file( config_file ) ) and os.path.exists( os.getcwd() + "/" + row['binary_file'] ):
                print "No need to rebuilt graph : configuration didn't change so loading from binary file"
                self.g = layer.MultimodalGraph(layers, str(row['binary_file']))
        else:
            if not row:
                print "This is the first time of launch: creating multimodal graph from scratch"
            elif row['md5'] != md5_of_file( file( config_file ) ):
                print "Configuration has changed since last launch. Rebuilding multimodal graph"
                if os.path.exists( os.getcwd() + "/" + row['binary_file'] ) :
                    os.remove(os.getcwd() + "/" + row['binary_file'])   
                self.config_table.delete().execute()
                self.session.commit()
            elif not os.path.exists( os.getcwd() + "/" + row['binary_file'] ) :
                print "Configuration has not changed since last launch but binary file is missing. Rebuilding multimodal graph"
                self.config_table.delete().execute()
                self.session.commit()
            self.g = layer.MultimodalGraph( layers )
            for i in same_nodes_connection_array:
                self.g.connect_same_nodes( i['layer1']['layer'],i['layer2']['layer'],i['property'] )
            for i in nearest_nodes_connection_array:
                self.g.connect_nearest_nodes( i['layer1']['layer'],i['layer2']['layer'],i['cost'], i['cost2'] )
            for i in nodes_list_connection_array:
               try:
                   i['node_list']['url_api']
                   print 'Assuming that the nodes list are bike stations'
                   for j in self.bike_stations:
                       if j.url == i['node_list']['url_api']:
                            self.g.connect_nodes_from_list( i['layer1']['layer'],i['layer2']['layer'],j.stations,i['cost1'],i['cost2'] )
                            break
               except KeyError:
                   try:
                       i['node_list']['layer']
                       print 'Assuming that the nodes list is a public transport layer'
                       self.g.connect_nodes_from_list( i['layer1']['layer'],i['layer2']['layer'],i['node_list']['layer'].nodes(),i['cost1'],i['cost2'] )
                   except KeyError:
                       raise NameError('Can not connect layers from the node list')
            md5_config_checksum = md5_of_file( file( config_file ) )
            self.g.set_id( md5_config_checksum )
            self.g.save( md5_config_checksum + '.dump' )
            i = self.config_table.insert()
            i.execute({'config_file': config_file, 'md5': md5_config_checksum, 'binary_file': md5_config_checksum + '.dump'})   

    @cherrypy.expose
    def path(self, slon, slat, dlon, dlat, time):
        start = self.g.match( 'Street', float(slon), float(slat))
        dest = self.g.match( 'Street', float(dlon), float(dlat))
        date = self.analyse_date( time )
        print "Searching path from {0} to {1} at time {2} on day {3}".format(start, dest, date['seconds'], date['days'])
        #res = mumoro.cap_jj_nf( self.g.graph ).visualization()
        #iso = mumoro.isochrone( self.g.graph, mumoro.foot_subway_dfa(), start, 1200 )
        #res = iso.visualization()
        
        #res = mumoro.rectangle_containing( self.g.graph, start, dest, 0.015 ).visualization()
        
        res = self.muparo( start, dest, date['seconds'], date['days'], self.g.graph )
        print res.a_nodes.size()
        return self.resultToGeoJson( res )
    
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
        res = mumoro.show_point_to_point(graph, dest, start, mumoro.car_dfa() )
        #res = mumoro.show_shared_path( graph, start, dest, 306 )
        #res = mumoro.show_car_sharing(graph, start, dest, 278790, 112254, mumoro.pt_foot_dfa(), mumoro.car_dfa())
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
            edges = self.g.graph.listEdges()
        elif restriction == 'Bus':
            edges = self.g.graph.listEdges(mumoro.BusEdge)
        elif restriction == 'Bike':
            edges = self.g.graph.listEdges(mumoro.BikeEdge)
        elif restriction == 'Foot':
            edges = self.g.graph.listEdges(mumoro.FootEdge)
        elif restriction == 'Car':
            edges = self.g.graph.listEdges(mumoro.CarEdge)
        elif restriction == 'Subway':
            edges = self.g.graph.listEdges(mumoro.SubwayEdge)
        elif restriction == 'Transfer':
            edges = self.g.graph.listEdges(mumoro.TransferEdge)
        elif restriction == 'Tram':
            edges = self.g.graph.listEdges(mumoro.TramEdge)
        return self.edgesToFeatures(edges)

    def edgesToFeatures(self, edges):
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
        for edge_id in edges:
            src_coord = self.g.coordinates(self.g.graph.source(edge_id))
            target_coord = self.g.coordinates(self.g.graph.target(edge_id))
            feature = {'type': 'feature'}
            geometry = {'type': 'Linestring'}
            coordinates = []

            connection = {
                    'type': 'feature',
                    'geometry': {
                        'type': 'Linestring',
                        'coordinates': [[src_coord[0], src_coord[1]], [target_coord[0], target_coord[1]]]
                        },
                    'properties': { 'layer': EdgeTypesToString[self.g.graph.map(edge_id).type] }
                    }
            features.append(connection);
        
        point = {
            'type': 'feature',
            'geometry': {
                'type': 'Point',
                'coordinates': [1.28873327891, 43.6730240687]
                },
            'properties': { 'layer': 'MeetingPt' }
        }
                
        features.append(point)
        p_str['features'] = features
        ret['paths'].append(p_str)
        print ret
        return json.dumps(ret)    
    
    def resultToGeoJson(self, res):
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
            src_coord = self.g.coordinates(self.g.graph.source(edge_id))
            target_coord = self.g.coordinates(self.g.graph.target(edge_id))
            feature = {'type': 'feature'}
            geometry = {'type': 'Linestring'}
            coordinates = []

            connection = {
                    'type': 'feature',
                    'geometry': {
                        'type': 'Linestring',
                        'coordinates': [[src_coord[0], src_coord[1]], [target_coord[0], target_coord[1]]]
                        },
                    'properties': { 'layer': EdgeTypesToString[self.g.graph.map(edge_id).type] }
                    }
            features.append(connection);
        
        for (nodes, layer) in ((res.a_nodes, 'PointA'), (res.b_nodes, 'PointB'), (res.c_nodes, 'PointC')):
            for node_id in nodes:
                node = self.g.graph.mapNode(node_id)
                feature = {
                    'type': 'feature',
                    'geometry': {
                        'type': 'Point',
                        'coordinates': [node.lon, node.lat]
                        },
                    'properties': { 'layer': layer }
                    }
                features.append(feature)
                    

        p_str['features'] = features
        ret['paths'].append(p_str)
        return json.dumps(ret)    

EdgeTypesToString = [ 'Foot', 'Bike', 'Car', 'Subway', 'Bus', 'Tram', 'Transfer', 'Unknown', 'All' ]
StringToEdgeType = { 'Foot': mumoro.FootEdge, 'Bike': mumoro.BikeEdge, 'Car': mumoro.CarEdge, 
                     'Subway': mumoro.SubwayEdge, 'Bus': mumoro.BusEdge, 'Tram': mumoro.TramEdge, 
                     'Transfer': mumoro.TransferEdge, 'Unknown': mumoro.UnknownEdgeType, 'All': mumoro.WhateverEdge }

total = len( sys.argv )
if total != 2:
    sys.exit("Usage: python server.py {config_file.py}")
if not os.path.exists( os.getcwd() + "/" + sys.argv[1] ):
    raise NameError('Configuration file does not exist')
exec( file( sys.argv[1] ) )
cherrypy.config.update({
    'tools.encode.on': True,
    'tools.encode.encoding': 'utf-8',
    'tools.decode.on': True,
    'tools.trailing_slash.on': True,
    'tools.staticdir.root': os.path.abspath(os.path.dirname(__file__)) + "/web/",
    'server.socket_port': listening_port,
    'server.socket_host': '0.0.0.0'
})

cherrypy.quickstart(Mumoro(db_type + ":///" + db_params,sys.argv[1],admin_email,web_url), '/', config={
    '/': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': 'static'
       },
})

def main():
    print "Goodbye!"
    

