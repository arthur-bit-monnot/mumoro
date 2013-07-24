# -*- coding: utf-8 -*-

# Copyright : Université Toulouse 1 (2010)

# Contributors : 
# Tristram Gräbener
# Odysseas Gabrielides
# Arthur Bit-Monnot (arthur.bit-monnot@laas.fr)

# This software is a computer program whose purpose is to [describe
# functionalities and technical features of your software].

# This software is governed by the CeCILL-B license under French law and
# abiding by the rules of distribution of free software.  You can  use, 
# modify and/ or redistribute the software under the terms of the CeCILL-B
# license as circulated by CEA, CNRS and INRIA at the following URL
# "http://www.cecill.info". 

# As a counterpart to the access to the source code and  rights to copy,
# modify and redistribute granted by the license, users are provided only
# with a limited warranty  and the software's author,  the holder of the
# economic rights,  and the successive licensors  have only  limited
# liability. 

# In this respect, the user's attention is drawn to the risks associated
# with loading,  using,  modifying and/or developing or reproducing the
# software by the user in light of its specific status of free software,
# that may mean  that it is complicated to manipulate,  and  that  also
# therefore means  that it is reserved for developers  and  experienced
# professionals having in-depth computer knowledge. Users are therefore
# encouraged to load and test the software's suitability as regards their
# requirements in conditions enabling the security of their systems and/or 
# data to be ensured and,  more generally, to use and operate it in the 
# same conditions as regards security. 

# The fact that you are presently reading this means that you have had
# knowledge of the CeCILL-B license and that you accept its terms. 

from sqlalchemy import *

class Metadata(object):
    def __init__(self, name, node_or_edge, origin):
        self.name = name
        self.node_or_edge = node_or_edge
        self.origin = origin

class Node(object):
    def __init__(self, id, lon, lat, the_geom = ""):
        self.original_id = id
        self.lon = lon
        self.lat = lat
        self.the_geom = the_geom

class PT_Node(object):
    def __init__(self, id, lon, lat, route, stop_area, linkable, the_geom = ""):
        self.original_id = id
        self.lon = lon
        self.lat = lat
        self.route = route
        self.the_geom = the_geom
        self.stop_area = stop_area
        self.linkable = linkable

class PT_Service(object):
    def __init__(self, id, services):
        self.id = id
        self.services = services

class Edge(object):
    def __init__(self, id, source, target, length, car, car_rev, bike, bike_rev, foot, the_geom = ""):
        self.original_id = id
        self.source = source
        self.target = target
        self.length = length
        self.car = car
        self.car_rev = car_rev
        self.bike = bike
        self.bike_rev = bike_rev
        self.foot = foot
        self.the_geom = the_geom

class PT_Edge(object):
    def __init__(self, source, target, length, duration_type, start_secs, arrival_secs, duration, services, mode, line):
        self.source = source
        self.target = target
        self.length = length
        # If it is a frequency. In this case, start_secs (resp. arrival_secs) is the begining (resp. end) of the period.
        # duration is the time of transit between two stations in this interval
        self.duration_type = duration_type
        self.start_secs = start_secs
        self.arrival_secs = arrival_secs
        self.duration = duration
        self.services = services
        self.mode = mode
        self.line = line

class PT_Line(object):
    def __init__(self, code, short_name, long_name, color, text_color, desc):
        self.code = code
        self.short_name = short_name
        self.long_name = long_name
        self.color = color
        self.text_color = text_color
        self.desc = desc

class PT_StopArea(object):
    def __init__(self, code, name):
        self.code = code
        self.name = name

def create_nodes_table(id, metadata):
    table = Table(id, metadata, 
            Column('id', Integer, primary_key = True),
            Column('original_id', String, index = True),
            Column('elevation', Integer),
            Column('lon', Float, index = True),
            Column('lat', Float, index = True),
            Column('the_geom', String),
            )
    metadata.create_all()
    return table

def create_pt_nodes_table(id, metadata, stop_areas_table):
    table = Table(id, metadata, 
            Column('id', Integer, primary_key = True),
            Column('original_id', String, index = True),
            Column('lon', Float, index = True),
            Column('lat', Float),
            Column('the_geom', String),
            Column('route', String),
            Column('stop_area', Integer, ForeignKey(stop_areas_table + ".id")),
            Column('linkable', Boolean)
            )
    metadata.create_all()
    return table

def create_services_table(id, metadata):
    table = Table(id, metadata,
            Column('id', Integer, primary_key = True),
            Column('services', String)
            )
    metadata.create_all()
    return table

def create_edges_table(id, metadata):
    table = Table(id, metadata,
            Column('id', Integer, primary_key = True),
            Column('source', Integer, index = True),
            Column('target', Integer, index = True),
            Column('length', Float),
            Column('car', Integer),
            Column('car_rev', Integer),
            Column('bike', Integer),
            Column('bike_rev', Integer),
            Column('foot', Integer),
            Column('the_geom', String),
            )
    metadata.create_all()
    return table
            
def create_pt_edges_table(id, metadata, services_table, lines_table):
    table = Table(id, metadata,
            Column('id', Integer, primary_key = True),
            Column('source', Integer, index = True),
            Column('target', Integer, index = True),
            Column('length', Float),
            Column('duration_type', Integer),
            Column('start_secs', Integer),
            Column('arrival_secs', Integer),
            Column('duration', Integer),
            Column('services', Integer,  ForeignKey(services_table + ".id")),
            Column('mode', Integer),
            Column('line', Integer, ForeignKey(lines_table + ".id"))
            )
    metadata.create_all()
    return table

def create_pt_lines_table(id, metadata):
    table = Table(id, metadata,
            Column('id', Integer, primary_key = True),
            Column('code', String),
            Column('short_name', String),
            Column('long_name', String),
            Column('color', String),
            Column('text_color', String),
            Column('desc', String)
            )
    metadata.create_all()
    return table

def create_pt_stop_areas_table(id, metadata):
    table = Table(id, metadata,
            Column('id', Integer, primary_key = True),
            Column('code', String, index = True),
            Column('name', String)
            )
    metadata.create_all()
    return table
