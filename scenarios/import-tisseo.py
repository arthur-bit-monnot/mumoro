#MUMORO CONFIGURATION BEFORE LAUNCHING SERVER

data_dir = "/home/arthur/LAAS/Data/"

#Database type, choose among : 'sqlite', 'mysql', 'postgres', 'oracle', 'mssql', and 'firebird'
db_type = 'sqlite'

#Database connexion URL
#For user oriented databases : 'username:password@host:port/database'
#Port can be excluded (default one depending on db_type will be used) : 'username:password@host/database'
#For SQLiTE : 'file_name.db' for relative path or absolute : '/data/guidage/file_name.db'
db_params = data_dir + 'DB/midi-pyrennees.db'

#Load street data from (compressed or not) osm file(s)
#-----------------------------------------------------


#Load bike service from an API URL (Don't forget to add http://) with required valid params (depending on each API)
#------------------------------------------------------------------------------------------------------------------

#Loads muncipal data file and inserts it into database.
#starting_date & end_date in this format : 'YYYYMMDD' Y for year's digists, M for month's and D for day's
#starting_date and end_date MUST be defined if municipal data is imported
#------------------------------------------------------------------------------------------------------------------
start_date = '20130208'
end_date = '20130310'

tisseo_data = import_gtfs_data( data_dir + 'GTFS/tisseo-raw.zip', 'Tisseo' )
