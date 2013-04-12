#MUMORO CONFIGURATION BEFORE LAUNCHING SERVER

data_dir = "/home/arthur/LAAS/Data/"

#Database type, choose among : 'sqlite', 'mysql', 'postgres', 'oracle', 'mssql', and 'firebird'
db_type = 'sqlite'

#Database connexion URL
#For user oriented databases : 'username:password@host:port/database'
#Port can be excluded (default one depending on db_type will be used) : 'username:password@host/database'
#For SQLiTE : 'file_name.db' for relative path or absolute : '/data/guidage/file_name.db'
db_params = data_dir + 'DB/sud-ouest.db'

start_date = '20130208'
end_date = '20130308'

osm = import_street_data ( [ data_dir + "OSM/mp-filtered.osm", 
                             data_dir + "OSM/aquitaine-filtered.osm" ] )

street = mixed_street_layer( data = osm, name = 'Street', color = '#842DCE')

#Loads muncipal data file and inserts it into database.
#starting_date & end_date in this format : 'YYYYMMDD' Y for year's digists, M for month's and D for day's
#starting_date and end_date MUST be defined if municipal data is imported
#------------------------------------------------------------------------------------------------------------------
start_date = '20130410'
end_date = '20130510'

tisseo_data = import_gtfs_data( data_dir + 'GTFS/tisseo-latest.zip', 'Tisseo' )
cub_bus_data = import_gtfs_data( data_dir + 'GTFS/cub-bus.zip', 'Cub-Bus' )
cub_tram_data = import_gtfs_data( data_dir + 'GTFS/cub-tram.zip', 'Cub-Tram' )


tisseo = public_transport_layer( data = tisseo_data, name = 'Metro Tisseo', color = '#4CC417' )
cub_bus = public_transport_layer( data = cub_bus_data, name = 'CUB Bus', color = '#4CC417' )
cub_tram = public_transport_layer( data = cub_tram_data, name = 'CUB Tram', color = '#4CC417' )

transfer = transferEdge( duration = 60, mode_change = True )

connect_layers_on_nearest_nodes( tisseo, street, transfer )
connect_layers_on_nearest_nodes( cub_bus, street, transfer )
connect_layers_on_nearest_nodes( cub_tram, street, transfer )

paths( street, street, [  ] )

#Administrator valid email
#REQUIRED for geocoding services, if empty the service will NOT work
#-------------------------
admin_email = 'arthur.bit-monnot@laas.fr'

#Website valid URL : 'http://url/' example 'http://mumoro.openstreetmap.fr/'
#REQUIRED for urls generating (allowing you to send the url to a friend and to find the same route)
#---------------------------------------------------------------------------
web_url = 'http://localhost:3001/' 

#Listening port
#Check that it is free and port-fordwarded if behind a router
#---------------
listening_port = 3001
