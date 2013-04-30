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

from lib.datastructures import *
import core.mumoro as mumoro

from sqlalchemy import *
from sqlalchemy.orm import *
 
class NotAccessible(Exception):
    pass

class NoLength(Exception):
    pass

class DataIncoherence(Exception):
    pass

 
def duration(length, property, type):
    if not length and length != 0.0:
        raise NoLength()
        
    if type == mumoro.FootEdge:
        if property == 0:
            raise NotAccessible()
        else:
            return length * 3.6 / 5
    elif type == mumoro.BikeEdge:
        if property == 0:
            raise NotAccessible()
        else:
            return length * 3.6 / 15
    elif type == mumoro.CarEdge:
        if property == 1:
            return length * 3.6 / 15
        elif property == 2:
            return length * 3.6 / 20
        elif property == 3:
            return length * 3.6 / 30
        elif property == 4:
            return length * 3.6 / 70
        elif property == 5:
            return length * 3.6 / 80
        elif property == 6:
            return length * 3.6 / 100
        else:
            raise NotAccessible()
    else:
        raise NotAccessible()
 
class BaseLayer(object):
    def __init__(self, name, data, metadata):
        self.data = data
        self.name = name
        self.metadata = metadata
        self.nodes_table = Table(data['nodes'], metadata, autoload = True)
        self.edges_table = Table(data['edges'], metadata, autoload = True)
        self.count = select([func.max(self.nodes_table.c.id)]).execute().first()[0] + 1

    def map(self, o_id):
        s = self.nodes_table.select(self.nodes_table.c.original_id==o_id)
        rs = s.execute()
        result = None
        for row in rs:
            result = row
        if result:
            return result[0] + self.offset
        else:
            print "Unable to find id {0}".format(o_id)
            return None

    def borders(self):
        max_lon = select([func.max(self.nodes_table.c.lon, type_=Float )]).execute().first()[0]    
        min_lon = select([func.min(self.nodes_table.c.lon, type_=Float )]).execute().first()[0]
        max_lat = select([func.max(self.nodes_table.c.lat, type_=Float )]).execute().first()[0]    
        min_lat = select([func.min(self.nodes_table.c.lat, type_=Float )]).execute().first()[0]
        return {'max_lon': max_lon,'min_lon': min_lon,'max_lat':max_lat,'min_lat':min_lat}

    def average(self):
        avg_lon = select([func.avg(self.nodes_table.c.lon, type_=Float )]).execute().first()[0]    
        avg_lat = select([func.avg(self.nodes_table.c.lat, type_=Float )]).execute().first()[0]
        return {'avg_lon':avg_lon, 'avg_lat':avg_lat }
 
    def match(self, ln, lt, epsilon = 0.002):
        ln = float(ln)
        lt = float(lt)
        res = self.nodes_table.select(
                (self.nodes_table.c.lon >= (ln - epsilon)) &
                (self.nodes_table.c.lon <= (ln + epsilon)) &
                (self.nodes_table.c.lat >= (lt - epsilon)) &
                (self.nodes_table.c.lat <= (lt + epsilon)),
                order_by = ((self.nodes_table.c.lon - ln) * (self.nodes_table.c.lon -ln)) + ((self.nodes_table.c.lat - lt) * (self.nodes_table.c.lat - lt))
                ).execute().first()
            
        if res:
            return res.id + self.offset
        else:
            return None

    def nearest(self, ln, lt):
        print "Trying to match {0}, {1}".format(ln, lt)
        nearest = None
        epsilon = 0.002
        while not nearest and epsilon < 0.008:
            nearest = self.match(ln, lt, epsilon)
            epsilon += 0.001
        return nearest
 
    def coordinates(self, nd):
        res = self.nodes_table.select(self.nodes_table.c.id == (nd - self.offset)).execute().first()    
        if res:
            return (res.lon, res.lat, res.original_id, self.name)
        else:
            print "Unknow node {0} on layer {1}, offset ".format(nd, self.name, self.offset)
 
    def nodes(self):
        for row in self.nodes_table.select().execute():
            yield row  

# A street layer containing only Car or Foot or Bike (depending on the value of mode)
class Layer(BaseLayer):
    def __init__(self, name, mode, data, metadata):
        super(Layer, self).__init__(name, data, metadata)
        self.mode = mode
               
    def edges(self):
        for edge in self.edges_table.select().execute():
            e = mumoro.Edge()
            e.length = edge.length
            if self.mode == mumoro.Foot:
                property = edge.foot
                property_rev = edge.foot
                e.type = mumoro.FootEdge
            elif self.mode == mumoro.Bike:
                property = edge.bike
                property_rev = edge.bike_rev
                e.type = mumoro.BikeEdge
            elif self.mode == mumoro.Car:
                property = edge.car
                property_rev = edge.car_rev
                e.type = mumoro.CarEdge
            else:
                property = 0
                property_rev = 0
                e.type = mumoro.UnkwownEdgeType
            
            node1 = self.map(edge.source)
            node2 = self.map(edge.target)
            try:
                dur = duration(e.length, property, e.type)
                e.duration = mumoro.Duration(dur)
                e.elevation = 0
              #  if self.mode == mumoro.Bike:
              #      e.elevation = max(0, target_alt - source_alt)
                yield {
                    'source': node1,
                    'target': node2,
                    'properties': e
                    }
            except NotAccessible:
                pass
 
            try:
                dur = duration(e.length, property_rev, e.type)
                e.duration = mumoro.Duration(dur)
                e.elevation = 0
#                if self.mode == mumoro.Bike:
#                    e.elevation = max(0, source_alt - target_alt)
                yield {
                    'source': node2,
                    'target': node1,
                    'properties': e,
                    }
            except NotAccessible:
                pass
            except NoLength:
                print "Error no length : ("+str(source)+", "+str(dest)+") "
                pass
 
# A street layer with mixed bike, foot and car
class MixedStreetLayer(BaseLayer):
    def __init__(self, name, data, metadata):
        super(MixedStreetLayer, self).__init__(name, data, metadata)
               
    def edges(self):
        
        count_total = 0
        count_ko = 0

        for edge in self.edges_table.select().execute():
            source = self.map(edge.source)
            dest = self.map(edge.target)
            
            # forward arc
            properties = [ {'prop': edge.foot, 'type': mumoro.FootEdge}, 
                           {'prop': edge.bike, 'type': mumoro.BikeEdge}, 
                           {'prop': edge.car,  'type': mumoro.CarEdge} ]
                           
            for property in properties:
                count_total += 1
                if count_total % 50000 == 0:
                    print "Treated edges : " + str(count_total)
                try:
                    dur = duration(edge.length, property['prop'], property['type'])
                    yield {
                        'source': source,
                        'target': dest,
                        'properties': { 'road_edge': True, 
                                        'type': property['type'], 
                                        'length': edge.length, 
                                        'duration': dur } 
                        }
                except NotAccessible:
                    pass
                except NoLength:
                    count_ko += 1
                    print "Error no length : ("+str(source)+", "+str(dest)+") "+str(count_ko) + " / " + str(count_total)
                    pass
 
            # backward arc
            properties = [ {'prop': edge.foot, 'type': mumoro.FootEdge}, 
                           {'prop': edge.bike_rev, 'type': mumoro.BikeEdge}, 
                           {'prop': edge.car_rev,  'type': mumoro.CarEdge} ]
                           
            for property in properties:
                count_total += 1
                if count_total % 50000 == 0:
                    print "Treated edges : " + str(count_total)
                    
                try:
                    dur = duration(edge.length, property['prop'], property['type'])
                    
                    yield {
                        'source': dest,
                        'target': source,
                        'properties': { 'road_edge': True, 
                                        'type': property['type'], 
                                        'length': edge.length, 
                                        'duration': dur }
                        }
                except NotAccessible:
                    pass
                except NoLength:
                    count_ko += 1
                    print "Error no length : ("+str(source)+", "+str(dest)+") "+str(count_ko) + " / " + str(count_total)
                    pass
 
class GTFSLayer(BaseLayer):
    """A layer for public transport described by the General Transit Feed Format"""
 
    def __init__(self, name, data, metadata):
        super(GTFSLayer, self).__init__(name, data, metadata)
        self.services = Table(data['services'], metadata, autoload = True)
        self.mode = mumoro.PublicTransport
 
    def edges(self):
        for row in self.edges_table.select().execute():
            services = self.services.select(self.services.c.id == int(row.services)).execute().first().services
            yield {
                    'source': row.source + self.offset,
                    'target': row.target + self.offset,
                    'duration_type': row.duration_type,
                    'departure': row.start_secs,
                    'arrival': row.arrival_secs,
                    'duration': row.duration,
                    'services': services,
                    'type': row.mode
                    }
 
        # Connects every node corresponding to a same stop:
        # if a stop is used by 3 routes, the stop will be represented by 3 nodes
        n1 = self.nodes_table.alias()
        n2 = self.nodes_table.alias()
        res = select([n1.c.id,n2.c.id], (n1.c.original_id == n2.c.original_id) & (n1.c.route != n2.c.route)).execute()
        count = 0
        for r in res:
            count += 1
            yield {
                    'source': r[0] + self.offset,    
                    'target': r[1] + self.offset,
                    'duration_type': mumoro.ConstDur,
                    'departure': 0,
                    'arrival': 0,
                    'duration': 60,
                    'services': "",
                    'type': mumoro.TransferEdge
                    }
        print "{0} transfer edge inserted".format(count)
 

class MultimodalGraph(object):
    def __init__(self, layers, id, filename = None):
        nb_nodes = 0
        self.node_to_layer = []
        self.layers = layers
        for l in layers:
            l.offset = nb_nodes
            nb_nodes += l.count
            self.node_to_layer.append((nb_nodes, l.name))
            print "Layer " + l.name + " for nodes from " + str(l.offset) +" to "+ str(nb_nodes - 1)
 
        if filename:
            self.graph_facto = mumoro.GraphFactory(filename)
        else:
            self.graph_facto = mumoro.GraphFactory(nb_nodes)
            self.graph_facto.set_id(id)
            
            count = 0
            for l in layers:
                for e in l.edges():
                    if e.has_key('properties') and e['properties']['road_edge']:
                        self.graph_facto.add_road_edge(e['source'], e['target'], e['properties']['type'], int(e['properties']['duration']))
                        count += 1
                    elif e.has_key('properties'):
                        if self.graph_facto.add_public_transport_edge(e['source'], e['target'], e['properties']['type'], int(e['properties']['duration'])):
                            count += 1
                    else:
                        if self.graph_facto.add_public_transport_edge(e['source'], e['target'], e['duration_type'], e['departure'], e['arrival'], e['duration'], 
                                                            str(e['services']), e['type']):
                            count += 1
                print "On layer {0}, {1} edges, {2} nodes".format(l.name, count, l.count)
                
                for n in l.nodes():
                    self.graph_facto.set_coord(n.id + l.offset, n.lon, n.lat)

            print "The multimodal graph has been built and has {0} nodes and {1} edges".format(nb_nodes, count)
 
    def graph(self):
        return self.graph_facto.get()
 
    def save(self, filename):
        self.graph_facto.save(filename)

    #def load(self, filename):
        #self.graph_facto.load(filename)
 
    def layer(self, node):
        for l in self.node_to_layer:
            if int(node) <= l[0]: # nodes from 1 to l[0] are in layer 0, etc
                return l[1]
        print sys.exc_info()[0]
        print "Unable to find the right layer for node {0}".format(node)
        print self.node_to_layer
 
    def coordinates(self, node):
        name = self.layer(node)
        for l in self.layers:
            if l.name == name:
                return l.coordinates(node)
        print sys.exc_info()[0]
        print "Unknown node: {0} on layer: {1}".format(node, name)
 
    def match(self, name, lon, lat):
        for l in self.layers:
            if l.name == name:
                return l.nearest(lon, lat)

 
    def connect_same_nodes(self, layer1, layer2, property):
        count = 0
        for n1 in layer1.nodes():
            n2 = layer2.map(n1.original_id)
            if n2:
                self.graph_facto.add_edge(n1.id + layer1.offset, n2, property)
                count += 1
                print count
        return count

    def connect_same_nodes_random(self, layer1, layer2, property, freq):
        count = 0
        for n1 in layer1.nodes():
            n2 = layer2.map(n1.original_id)
            if n2 and count % freq == 0:
                self.graph_facto.add_edge(n1.id + layer.offset, n2, property)
                count += 1
        return count


    def connect_nodes_from_list(self, layer1, layer2, list, property, property2 = None):
        count = 0
        if property2 == None:
            property2 = property
        for coord in list:
            n1 = layer1.match(coord['lon'], coord['lat'])
            n2 = layer2.match(coord['lon'], coord['lat'])
            if n1 and n2:
                self.graph_facto.add_edge(n1, n2, property)
                self.graph_facto.add_edge(n2, n1, property2)
                count += 2
            else:
                print "Uho... no connection possible"
        return count


    def connect_nearest_nodes(self, layer1, layer2, property, property2 = None):
        count = 0
        if property2 == None:
            property2 = property
        for n in layer1.nodes():
            # Only connect nodes flaged as linkable
            if not n.linkable:
                continue
            nearest = layer2.nearest(n.lon, n.lat)
            if nearest:
                self.graph_facto.add_public_transport_edge(n.id + layer1.offset, nearest, property['duration'], property['type'])
                self.graph_facto.add_public_transport_edge(nearest, n.id + layer1.offset, property2['duration'], property2['type'])
                count += 2
        return count
 
