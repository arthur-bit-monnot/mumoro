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

import bikestations
import psycopg2 as pg
import time

class StatsBikes:
    conn = None
    def __init__(self):
	self.addData()
    def createTable(self):
	pass
    def addData(self):
	s = bikestations.VeloStar(False)
        try:
            tmp = "dbname=guidage user=guidage"
            self.conn = pg.connect( tmp )
        except:
            print "I am unable to connect to the database"
        cur = self.conn.cursor()
        query = "TRUNCATE TABLE bike_stations"
	try:        
	    cur.execute( query )
	except Exception as ex:
            print "I am unable to empty data from the database"
            print ex
        for i in s.stations:
            self.addStationData(i)
        self.conn.commit()
        self.conn.close()
    def addStationData(self,s):
	cur = self.conn.cursor()
        day = time.localtime(time.time())[6]
        chrone = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
        query = "INSERT INTO bike_stats (\"idDay\", \"idStation\", \"avSlots\", \"avBikes\", \"chrone\") VALUES (%s, %s, %s, %s, %s)"
	try:        
	    cur.execute( query , [ day, s['num'], s['availableSlots'], s['availableBikes'], chrone ] )
	except Exception as ex:
            print "I am unable to insert bike stations stats data into the database"
            print ex
        query = "INSERT INTO bike_stations (\"id_station\", \"av_bikes\", \"av_slots\", \"name\", \"district_name\", \"lon\", \"lat\", \"chrone\") VALUES (%s, %s, %s, %s, %s, %s, %s, %s)"
	try:        
	    cur.execute( query , [ s['num'], s['availableBikes'], s['availableSlots'], s['name'], s['districtName'], s['lon'], s['lat'], chrone ] )
	except Exception as ex:
            print "I am unable to insert bike stations stats data into the database"
            print ex
if __name__ == "__main__":
    x=StatsBikes()
        
