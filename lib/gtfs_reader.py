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

from lib.core.mumoro import *
from datastructures import *
import sys
import transitfeed
import datetime
from optparse import OptionParser
from sqlalchemy.orm import mapper, sessionmaker



def distance(c1, c2):
    try:
        delta = c2[0] - c1[0]
        a = math.radians(c1[1])
        b = math.radians(c2[1])
        C = math.radians(delta)
        x = math.sin(a) * math.sin(b) + math.cos(a) * math.cos(b) * math.cos(C)
        distance = math.acos(x) # in radians
        distance  = math.degrees(distance) # in degrees
        distance  = distance * 60 # 60 nautical miles / lat degree
        distance = distance * 1852 # conversion to meters
        return distance;
    except:
        return 0
    
def gtfsToMumoroMode(gtfs_mode):
        if gtfs_mode == 0:
            return TramEdge
        elif gtfs_mode == 1: 
            return SubwayEdge
        elif gtfs_mode == 3:
            return BusEdge
        else: 
            return UnknownEdgeType

# Service string for a trip on every single day
every_day = "1"*128

class GtfsConverter:
    def __init__(self, filename, session, start_date, end_date):
        self.filename = filename
        self.session = session
        self.start_date = datetime.datetime.strptime(start_date, "%Y%m%d")
        self.end_date = datetime.datetime.strptime(end_date, "%Y%m%d")
        
        self.tt_map = {}
        self.freq_map = {}
        self.station_map = {}
        self.services_map = {}
        self.services_map[every_day] = len(self.services_map)
        self.s = transitfeed.Schedule()
        self.s.Load(filename)        

        #Start with mapping (route_id, stop_id) to an int
        self.count = 1
        self.routes_count = 0
        self.routes_map = {}
        
        self.stop_areas_count = 0
        self.stop_areas_map = {}
        
    def insert_station_node(self, trip, stop):
        if not self.station_map[trip.route_id].has_key(stop.stop_id):
            self.station_map[trip.route_id][stop.stop_id] = self.count
            physStop = self.s.GetStop(stop.stop_id)
            if not physStop.parent_station:
                self.session.add(PT_Node(stop.stop_id, stop.stop.stop_lon, stop.stop.stop_lat, trip.route_id, self.stop_areas_map[stop.stop_id], True))
            else:
                self.session.add(PT_Node(stop.stop_id, stop.stop.stop_lon, stop.stop.stop_lat, trip.route_id, self.stop_areas_map[physStop.parent_station], True))
            self.count +=1
    
    def insert_tt_node(self, trip, stop):
        if not self.tt_map[trip.route_id].has_key(stop.stop_id):
            self.tt_map[trip.route_id][stop.stop_id] = self.count
            physStop = self.s.GetStop(stop.stop_id)
            if not physStop.parent_station:
                self.session.add(PT_Node(stop.stop_id, stop.stop.stop_lon, stop.stop.stop_lat, trip.route_id, self.stop_areas_map[stop.stop_id], False))
            else:
                self.session.add(PT_Node(stop.stop_id, stop.stop.stop_lon, stop.stop.stop_lat, trip.route_id, self.stop_areas_map[physStop.parent_station], False))
            self.count +=1
            
            self.insert_station_node(trip, stop)
            station_id = self.station_map[trip.route_id][stop.stop_id]
            cur_stop = self.tt_map[trip.route_id][stop.stop_id]
            
            # Transfer edges from station to timetable edge : available all day with cost 0
            self.session.add(PT_Edge(station_id, cur_stop, 0, ConstDur, -1, -1, 0, self.services_map[every_day], TransferEdge, self.routes_map[trip.route_id]))
            self.session.add(PT_Edge(cur_stop, station_id, 0, ConstDur, -1, -1, 0, self.services_map[every_day], TransferEdge, self.routes_map[trip.route_id]))
        
    
    def insert_freq_node(self, trip, stop):
        if not self.freq_map[trip.route_id].has_key(stop.stop_id):
            self.freq_map[trip.route_id][stop.stop_id] = self.count
            physStop = self.s.GetStop(stop.stop_id)
            if not physStop.parent_station:
                self.session.add(PT_Node(stop.stop_id, stop.stop.stop_lon, stop.stop.stop_lat, trip.route_id, self.stop_areas_map[stop.stop_id], False))
            else:
                self.session.add(PT_Node(stop.stop_id, stop.stop.stop_lon, stop.stop.stop_lat, trip.route_id, self.stop_areas_map[physStop.parent_station], False))
            self.count +=1
            self.insert_station_node(trip, stop)
            station_id = self.station_map[trip.route_id][stop.stop_id]
            cur_stop = self.freq_map[trip.route_id][stop.stop_id]
            
            # Disambarking edge : available all day with cost 0
            self.session.add(PT_Edge(cur_stop, station_id, 0, ConstDur, -1, -1, 0, self.services_map[every_day], TransferEdge, self.routes_map[trip.route_id]))
    
    # Import a trip, adding the given offset to all departure/arrival times
    def load_trip(self, trip, service, mode, route, offset):
        prev_time = None
        prev_stop = None
        current_stop = None
        for stop in trip.GetStopTimes():
            self.insert_tt_node(trip, stop)
            current_stop = self.tt_map[trip.route_id][stop.stop_id]
            current_node = self.session.query(PT_Node).filter_by(original_id = stop.stop_id).first()
            if prev_stop != None:
                length = distance( (current_node.lon, current_node.lat), (prev_node.lon, prev_node.lat))
                self.session.add(PT_Edge(prev_stop, current_stop, length * 1.1, TimetableDur, prev_time+offset, stop.arrival_secs+offset, -1, service, mode, self.routes_map[route.route_id]))

            prev_node = current_node 
            prev_stop = current_stop
            prev_time = stop.departure_secs
            
            if self.count % 1000 == 0:
                self.session.flush()
            if self.count % 10000 == 0:
                print "Added {0} timetable elements".format(self.count)
        
    def convert(self):
        for route in self.s.GetRouteList():
            self.session.add(PT_Line(route.route_id, route.route_short_name, route.route_long_name, route.route_color, route.route_text_color, route.route_desc))
            self.routes_map[route.route_id] = self.routes_count
            self.routes_count += 1

        for stop in self.s.GetStopList():
            if not stop.parent_station:
                self.session.add(PT_StopArea(stop.stop_id, stop.stop_name))
                self.stop_areas_map[stop.stop_id] = self.stop_areas_count
                self.stop_areas_count += 1

        for trip in self.s.GetTripList():
            route =  self.s.GetRoute(trip.route_id)
            mode = gtfsToMumoroMode( route.route_type )
            service_period = self.s.GetServicePeriod(trip.service_id)
            services = ""
            delta = datetime.timedelta(days=1)
            date = self.start_date
            while date <= self.end_date:
                if service_period.IsActiveOn(date.strftime("%Y%m%d"), date_object=date):
                    services = "1" + services
                else:
                    services = "0" + services
                date += delta
            
            # don't treat trips that are not active during this period
            if not "1" in services:
                continue
            
            if not self.services_map.has_key(services):
                self.services_map[services] = len(self.services_map)
            service = self.services_map[services]

            if not self.tt_map.has_key(trip.route_id):
                self.tt_map[trip.route_id] = {}
                
            if not self.freq_map.has_key(trip.route_id):
                self.freq_map[trip.route_id] = {}
                
            if not self.station_map.has_key(trip.route_id):
                self.station_map[trip.route_id] = {}

            freqs = trip.GetFrequencyTuples()
            
            # single trip
            if not freqs: 
                self.load_trip(trip, service, mode, route, 0)
                    
            # trip with frequencies
            else:
                trip_start = trip.GetStopTimes()[0].GetTimeSecs()
                
                for freq in freqs:
                    self.load_trip(trip, service, mode, route, freq[0] - trip_start)
                    self.load_trip(trip, service, mode, route, freq[1] - trip_start)
                    period_start = freq[0]
                    period_end = freq[1] - freq[2]  #fin de l'interval = fin gtfs - temps entre deux trips
                    trip_freq = freq[2]    # temps entre deux départs
                    
                    prev_time = None
                    prev_stop = None
                    current_stop = None
                    #prev_period_start = period_start
                    #prev_period_end = period_end
                    
                    for stop in trip.GetStopTimes():
                        cur_period_start = period_start + stop.arrival_secs - trip_start
                        cur_period_end = period_end + stop.arrival_secs - trip_start
                        
                        self.insert_freq_node(trip, stop)
                        current_stop = self.freq_map[trip.route_id][stop.stop_id]
                        current_station = self.station_map[trip.route_id][stop.stop_id]
                        current_node = self.session.query(PT_Node).filter_by(original_id = stop.stop_id).first()
                        
                        # insert frequency edge from previous to current node during the interval [prev_period_start, prev_period_end]
                        # duration is the difference between departure from previous node and arrival in current
                        if prev_stop != None:
                            length = distance( (current_node.lon, current_node.lat), (prev_node.lon, prev_node.lat))
                            self.session.add(PT_Edge(prev_stop, current_stop, length * 1.1, FrequencyDur, 
                                                     prev_period_start, prev_period_end, stop.arrival_secs - prev_time, service, 
                                                     mode, self.routes_map[route.route_id]))

                        self.session.add(PT_Edge(current_station, current_stop, 0, FrequencyDur,
                                                 cur_period_start, cur_period_end, trip_freq, service,
                                                 TransferEdge, self.routes_map[route.route_id]))
                        prev_node = current_node 
                        prev_stop = current_stop
                        prev_time = stop.departure_secs
                        
                        prev_period_start = cur_period_start
                        prev_period_end = cur_period_end
                        
                        if self.count % 1000 == 0:
                            self.session.flush()
                        if self.count % 10000 == 0:
                            print "Added {0} timetable elements".format(self.count)

        for k, v in self.services_map.items():
            self.session.add(PT_Service(v, k))
        self.session.commit()


