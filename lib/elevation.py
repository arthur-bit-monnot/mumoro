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

import array
import math
import zipfile
import os.path
import urllib

def filename(lat, lon):
    if lat > 0 :
        filename = "N%02d" % math.trunc(lat)
    else:
        filename = "S%02d" % -math.trunc(lat - 1)
    if lon > 0 :
        filename += "E%03d.hgt" % math.trunc(lon)
    else:
        filename += "W%03d.hgt" % -math.trunc(lon - 1)
    return  filename


class Tile:
    nb_coords = 1201;
    def __init__(self, lat, lon):
        zf = zipfile.ZipFile(filename(lat, lon) + ".zip")
        self.data = array.array("h", zf.read(filename(lat, lon)))
        self.data.byteswap()

    def altitude(self, lat, lon):
        lon_dec = abs(lon) - math.trunc(abs(lon))
        lat_dec = abs(lat) - math.trunc(abs(lat))
        
        if lon > 0 :
            lon_idx = math.trunc(lon_dec * self.nb_coords)
        else:
            lon_idx = math.trunc((1 - lon_dec) * self.nb_coords -1)
        if lat > 0 :
            lat_idx = math.trunc((1 - lat_dec) * self.nb_coords - 1)
        else:
            lat_idx = math.trunc(lat_dec * self.nb_coords)

        return self.data[lat_idx * self.nb_coords + lon_idx]

class ElevationData:
    def __init__(self, continent):
        if continent not in ["Africa", "Australia", "Eurasia", "Islands", "North_America", "South_America"]:
            print "Error: unknow continent %s." % continent
            raise Exception
        self.tiles = {}
        self.continent = continent


    def altitude(self, lat, lon):
        fn = filename(lat, lon)
        if not self.tiles.has_key(fn):
            if not os.path.exists(fn + ".zip"):
#                url = "ftp://e0srp01u.ecs.nasa.gov/srtm/version2/SRTM3/%s/%s.zip" % (self.continent, fn)
                url = "http://dds.cr.usgs.gov/srtm/version2_1/SRTM3/%s/%s.zip" % (self.continent, fn)
                print "Tile not in cache. Downloading %s " %url
                urllib.urlretrieve(url, fn + ".zip")
                print "    Done!"
            self.tiles[fn] = Tile(lat, lon)
        return self.tiles[fn].altitude(lat, lon)
