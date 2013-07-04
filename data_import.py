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
#    © Université Toulouse 1 Capitole 2010
#    © Tristram Gräbener 2011
#    Author: Tristram Gräbener, Odysseas Gabrielides

import sys
from lib.core.mumoro import *
from lib.core import mumoro
import osm4routing
from lib import bikestations, gtfs_reader, kalkati_reader
import os.path
from lib.datastructures import *
from sqlalchemy import *
from sqlalchemy.orm import mapper, sessionmaker, clear_mappers
import datetime
import csv

data_dir = ""

street_data_array = []
municipal_data_array = []
kalkati_data_array = []
freq_data_array = []
bike_service_array = []


class Importer():
    def __init__(self,db_type,db_params, start_date, end_date):
        if not db_type or not db_params:
            raise NameError('Database connection parameters are empty')
        self.db_string = db_type + ":///" + db_params
        print "Connection string is : " + self.db_string        
        self.engine = create_engine(self.db_string)
        self.metadata = MetaData(bind = self.engine)
        self.mumoro_metadata = Table('metadata', self.metadata,
                Column('id', Integer, primary_key=True),
                Column('node_or_edge', String),
                Column('name', String),
                Column('origin', String)
                )
        self.metadata.create_all()
        mapper(Metadata, self.mumoro_metadata)
        Session = sessionmaker(bind=self.engine)
        self.session = Session()
        
        for f in street_data_array:
            self.import_osm( f )

        if municipal_data_array:
            is_date_valid( start_date )
            is_date_valid( end_date )
            for m in municipal_data_array:
                self.import_gtfs( m['file'], m['origin'], start_date, end_date, m['network'] )

        for b in bike_service_array:
            self.import_bike( b['url_api'], b['service_name'] )


        if kalkati_data_array:
            is_date_valid( start_date )
            is_date_valid( end_date )
            for m in kalkati_data_array:
                self.import_kalkati( m['file'], start_date, end_date, m['network'] )

        for f in freq_data_array:
            self.import_freq(f['line_name'], f['nodesf'], f['linesf'], start_date, end_date)



    def init_mappers(self):
        clear_mappers()
        mapper(Metadata, self.mumoro_metadata)

    def import_osm( self, street_data ):
        origin = str( street_data['origins'] )
        print "Adding street data from " + origin
        nodes = Metadata("OSM nodes", "Nodes", origin)
        edges = Metadata("OSM edges", "Edges", origin)
        self.session.add(nodes)
        self.session.add(edges)
        self.session.commit()
        for filename in street_data['files']:
            osm4routing.parse(filename, self.db_string, str(edges.id), str(nodes.id) ) 
            print "Done importing street data from " + filename
            print "---------------------------------------------------------------------"
        self.init_mappers()

    def import_gtfs(self, filename, origin, start_date, end_date, network_name = "GTFS"):
        print "Adding municipal data from " + filename
        print "From " + start_date + " to " + end_date + " for " + network_name + " network"

        stop_areas = Metadata(network_name, "StopAreas", origin)
        self.session.add(stop_areas)
        self.session.commit()
        mapper(PT_StopArea, create_pt_stop_areas_table(str(stop_areas.id), self.metadata))

        nodes2 = Metadata(network_name, "Nodes", origin)
        self.session.add(nodes2)
        self.session.commit()
        mapper(PT_Node, create_pt_nodes_table(str(nodes2.id), self.metadata, str(stop_areas.id)))
        
        services = Metadata(network_name, "Services", origin)
        self.session.add(services)
        self.session.commit()
        mapper(PT_Service, create_services_table(str(services.id), self.metadata))

        lines = Metadata(network_name, "Lines", origin)
        self.session.add(lines)
        self.session.commit()
        mapper(PT_Line, create_pt_lines_table(str(lines.id), self.metadata))

        edges2 = Metadata(network_name, "Edges", origin)
        self.session.add(edges2)
        self.session.commit()
        mapper(PT_Edge, create_pt_edges_table(str(edges2.id), self.metadata, str(services.id), str(lines.id)))
        self.session.commit()
        
        gtfs_converter = gtfs_reader.GtfsConverter(filename, self.session, start_date, end_date)
        gtfs_converter.convert()

        self.init_mappers()
        print "Done importing municipal data from " + filename + " for network '" + network_name + "'"
        print "---------------------------------------------------------------------"

#Loads an osm (compressed of not) file and insert data into database
def import_street_data( filenames ):
    street_data = { 'files': [], 'origins': [] }
    for filename in filenames:
        if os.path.exists( data_dir + filename ):
            street_data['files'].append( data_dir + filename )
            street_data['origins'].append( filename )
        elif os.path.exists( filename ):
            street_data['files'].append( filename )
            street_data['origins'].append( filename )
        else:
            raise NameError('File does not exist: '+'-'+ data_dir + '-'+filename)
    street_data_array.append( street_data )

# Loads public transport data from GTFS file format
def import_gtfs_data( filename, network_name = "Public transport"):
    if os.path.exists( data_dir + filename ):
        municipal_data_array.append( {'file': data_dir + filename, 'origin': filename, 'network': network_name } )
    elif os.path.exists( filename ):
        municipal_data_array.append( {'file': filename, 'origin': filename, 'network': network_name } )
    else:
        raise NameError('File does not exist: '+filename)
    
#Loads data from previous inserted data and creates a layer used in multi-modal graph
def street_layer(data, name, color, mode):
    pass

def mixed_street_layer( data, name, color ):
    pass

def public_transport_layer(data, name, color):
    pass

def paths( starting_layer, destination_layer, objectives ):
    pass


def set_starting_layer( layer ):
    pass

def set_destination_layer( layer ):
    pass

#Creates a transit cost variable, including the duration in seconds of the transit and if the mode is changed
def transferEdge( duration, mode_change ):
    pass

#Connects 2 given layers on same nodes with the given cost(s)
def connect_layers_same_nodes( layer1, layer2, cost ):
    pass

#Connect 2 given layers on a node list (arg 3 which should be the returned data from import_municipal_data or import_bike_service) with the given cost(s)
def connect_layers_from_node_list( layer1, layer2, node_list, cost, cost2 = None ):
    pass

#Connect 2 given layers on nearest nodes
def connect_layers_on_nearest_nodes( layer1 , layer2, cost, cost2 = None):
    pass

def is_date_valid( date ):
   date = datetime.datetime.strptime(date, "%Y%m%d")



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
    Importer(db_type,db_params, start_date, end_date)
    

