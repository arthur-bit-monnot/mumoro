from Builder import *
import sys, os

exec( file('Builder.py') )



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
    db_string = db_type + ":///" + db_params

    Builder( os.getcwd() + "/" + sys.argv[1], source='db', target=None ) 