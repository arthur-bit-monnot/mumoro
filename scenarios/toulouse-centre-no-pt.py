#MUMORO CONFIGURATION BEFORE LAUNCHING SERVER

data_dir = "/home/arthur/LAAS/Data/"

#Database type, choose among : 'sqlite', 'mysql', 'postgres', 'oracle', 'mssql', and 'firebird'
db_type = 'sqlite'

#Database connexion URL
#For user oriented databases : 'username:password@host:port/database'
#Port can be excluded (default one depending on db_type will be used) : 'username:password@host/database'
#For SQLiTE : 'file_name.db' for relative path or absolute : '/data/guidage/file_name.db'
db_params = data_dir + 'DB/toulouse-centre-no-pt.db'

start_date = '20130208'
end_date = '20130308'

osm_data = import_street_data (data_dir + "OSM/toulouse-centre.osm")

street = mixed_street_layer( data = osm_data, name = 'Street', color = '#842DCE')

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
