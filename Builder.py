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

class Builder:
    def __init__(self, data_dir, source, target=None ):
        #exec( file( data_dir + 'conf.py' ) )
        
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
        
        bin_dump_file = data_dir + 'graph.bin-dump'
        
        if source == 'bin-dump':
            if not os.path.exists( bin_dump_file ):
                print "Unable to find binary graph " + bin_dump_file
            else:
                print "Loading from binary file " + bin_dump_file
                self.g = layer.MultimodalGraph(layers, 'id', bin_dump_file)
        elif source == 'db':
            if not os.path.exists( bin_dump_file ):
                print 'Warning: the previous graph will be overwriten'
            
            self.config_table.delete().execute()
            self.session.commit()
            
            self.g = layer.MultimodalGraph( layers, bin_dump_file )
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

            if target == 'bin-dump' or source == 'db' and target == None:
                self.g.save( bin_dump_file )
                print 'Wrote binary graph dump to ' + bin_dump_file


    

