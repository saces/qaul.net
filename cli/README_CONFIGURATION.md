CLI Configuration
=================

To configure your qaul.net CLI client you have several options

* Over the web interface
* Configure data base via data base manager
* Configure data base via SQL on the CLI


Web Interface
-------------

To configure the qaul.net application via web interface, simply 
start the application and open http://localhost:8081/qaul.html
in your web browser.


Data Base Manager
-----------------

Start the qaul.net application at least once for that the data base
file is generated.
Open the data base file in an sqlite data base manager.

There are several free visual sqlite database managers:
* The firefox Add-on SQLite Manager.
* On Linux there is the 'SQLite database browser'.

The configuration options are to find in the data base table 'config'.


SQL via CLI
-----------

Start the qaul.net application at least once for that the data base
file is generated.

Install the sqlite3 command line interface program:

    # on Debian / Ubuntu enter
    sudo apt-get install sqlite3


Open and edit the data base:

    # Open the data base file qaullib.db
    sqlite3 qaullib.db
    # Close the data base
    .exit
    
    # configure user name
    SELECT value FROM 'config' WHERE key = "username";
    DELETE FROM 'config' WHERE key = "username";
    INSERT INTO 'config' ('key','type','value') VALUES ("username",1,"YourUserName");
    
    # configure interface language
    SELECT value FROM 'config' WHERE key = "locale";
    DELETE FROM 'config' WHERE key = "locale";
    INSERT INTO 'config' ('key','type','value') VALUES ("locale",1,"en");
    
    # configure IP
    SELECT value FROM 'config' WHERE key = "ip";
    DELETE FROM 'config' WHERE key = "ip";
    INSERT INTO 'config' ('key','type','value') VALUES ("ip",1,"10.11.12.13");

    # configure network interface manually
    SELECT value_int FROM 'config' WHERE key = "net.interface.manual";
    SELECT value FROM 'config' WHERE key = "net.interface.name";
    DELETE FROM 'config' WHERE key = "net.interface.manual";
    DELETE FROM 'config' WHERE key = "net.interface.name";
    INSERT INTO 'config' ('key','type','value','value_int') VALUES ("net.interface.manual",0,"",1);
    INSERT INTO 'config' ('key','type','value') VALUES ("net.interface.name",1,"eth0");
