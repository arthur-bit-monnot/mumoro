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

from datastructures import *
import sys
import datetime
from optparse import OptionParser
from sqlalchemy.orm import mapper, sessionmaker
import xml.sax
from xml.sax.handler import ContentHandler
import iso8601

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


def normalize_service(start, end, services, service_start):
    original = services
    start_delta = (start - service_start).days
    if start_delta < 0:
        services = services + ("0" * abs(start_delta))
    else:
        services = services[:-start_delta]
    end_delta = len(services) - (end - start).days
    if end_delta < 0:
        services = ("0" * abs(end_delta)) + services
    else:
        services = services[end_delta :]

    if len(services) != (end - start).days:
        print "Crapp!!!! {0} {1} {2} {3} {4} {5}".format(services, start_delta, end_delta, service_start, original, start)
    return services

class KalkatiHandler(ContentHandler): 
    def __init__(self, session, start_date, end_date):
#        self.nodes_insert = nodes.insert()
#        self.edges_insert = edges.insert()
        self.start = datetime.datetime.strptime(start_date, "%Y%m%d")
        self.end = datetime.datetime.strptime(end_date, "%Y%m%d")
        self.session = session
        self.company = {}
        self.stations = {}
        self.changes = []
        self.map = {}
        self.synonym = False
        self.count = 0

    def startElement(self, name, attrs): 
        if name == "Delivery": #[1..1]
            self.firstday = iso8601.parse_date(attrs["Firstday"])
            self.lastday = iso8601.parse_date(attrs["Lastday"])

        elif name == "Company": #[1..*]
            self.company[attrs["CompanyId"]] = attrs["Name"]

        elif name == "Country": #[1..*]
            pass

        elif name == "Timezone": #[1..*]
            pass

        elif name == "Period": #[1..*] in Timezone
            pass

        elif name == "Language": #[1..*]
            pass

        elif name == "Station" and not self.synonym: #[0..*]
            s = {}
            if attrs["Name"]:
                s["name"] = attrs["Name"]
            if attrs.has_key("X") and attrs.has_key("Y"):
                s["x"], s["y"] = float(attrs["X"]), float(attrs["Y"])
            else:
                print "Warning! No X/Y for station {0}, {1}".format(s["id"], s["name"].encode("iso-8859-1" ) )
            self.stations[attrs["StationId"]] = (s)

        elif name == "Trnsattr": #[0..*]
            pass

        elif name == "Trnsmode": #[0..*]
            pass

        elif name == "Synonym": #[0..*]
            self.synonym = True

        elif name == "Change":  #[0..*]
            c = {}
            c["service1"] = attrs["ServiceId1"]
            c["service2"] = attrs["ServiceId2"]
            if attrs["ChangeTime"]:
                c["change_time"] = attrs["ChangeTime"]

        elif name == "Timetbls": #[0..1]
            pass # It's just a container for Services

        elif name == "Service": #[0..*] in Timetbls
            self.service = attrs["ServiceId"]
            self.prev_stop = None
            self.prev_time = None

        elif name == "ServiceNbr": #[1..1] in Service
            self.trip = attrs["ServiceNbr"]
            if not self.map.has_key(self.trip):
                self.map[self.trip] = {}

        elif name == "ServiceValidity": # in Service
            self.footnote = attrs["FootnoteId"]

        elif name == "ServiceTrnsmode": # in Service
            self.mode = attrs["TrnsmodeId"]

        elif name == "ServiceAttribute": # in Service
            pass

        elif name == "Stop": #[1..*] in Service
            station = attrs["StationId"]
            if not self.map[self.trip].has_key(station):
                self.map[self.trip][station] = self.count
                self.count += 1
                self.session.add(PT_Node(station, self.stations[station]["x"],self.stations[station]["y"], self.trip))
            current_stop = self.map[self.trip][station]
            if attrs.has_key("Arrival"):
                arrival = int(attrs["Arrival"]) * 60
            else:
                arrival = int(attrs["Departure"]) * 60
            if attrs.has_key("Departure"):
                departure = int(attrs["Departure"]) * 60
            else:
                departure = int(attrs["Arrival"]) * 60


            if self.prev_stop:
                length = distance( (self.stations[station]["x"],self.stations[station]["y"]), (self.prev_lon, self.prev_lat))

                
                self.session.add(PT_Edge(
                        source = self.prev_stop,
                        target = current_stop,
                        length = length * 1.1,
                        start_secs = self.prev_time,
                        arrival_secs = arrival,
                        services = self.footnote,
                        mode = self.mode
                        ))
                    
            self.prev_stop = current_stop
            self.prev_time = departure
            self.prev_lon = self.stations[station]["x"]
            self.prev_lat = self.stations[station]["y"]

            if self.count % 100 == 0:
                self.session.flush()
            if self.count % 10000 == 0:
                print "Added {0} timetable elements".format(self.count)

        elif name == "Footnote":
            if attrs.has_key("Firstdate"):
                date = datetime.datetime.strptime(attrs["Firstdate"], "%Y-%m-%d")
            else:
                date = self.firstday
            
            services = normalize_service(self.start, self.end, attrs["Vector"], date)

            self.session.add(PT_Service(int(attrs["FootnoteId"]), services))
    def endElement(self, name):
        if name == "Synonym":
            self.synonym = False

    def endDocument(self):
        self.session.commit()

def convert(filename, session, start_date, end_date):
    handler = KalkatiHandler(session, start_date, end_date)
    session.commit()
    xml.sax.parse(filename, handler)

