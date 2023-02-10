CREATE DOMAIN IF NOT EXISTS uint2 AS int4
   CHECK(VALUE >= 0 AND VALUE < 65536);

CREATE TABLE IF NOT EXISTS server_details (
    SERVER_ID SERIAL PRIMARY KEY,
    ADDRESS inet NOT NULL,
    PORT uint2,
    UNIQUE (ADDRESS, PORT)
);

CREATE TABLE IF NOT EXISTS server_active (
    SERVER_ID int NOT NULL UNIQUE REFERENCES server_details (SERVER_ID) ON DELETE CASCADE,
    ACTIVE boolean NOT NULL,
    PRIMARY KEY (SERVER_ID)
);

CREATE TABLE IF NOT EXISTS discord_channel (
    CHANNEL_ID bigint NOT NULL UNIQUE PRIMARY KEY,
    GUILD_ID bigint NOT NULL,
    UNIQUE (CHANNEL_ID, GUILD_ID)
);

CREATE TABLE IF NOT EXISTS channel_server (
    SERVER_ID int NOT NULL REFERENCES server_details (SERVER_ID) ON DELETE CASCADE,
    CHANNEL_ID bigint NOT NULL REFERENCES discord_channel (CHANNEL_ID) ON DELETE CASCADE,
    MESSAGE_ID bigint DEFAULT NULL
);

CREATE OR REPLACE FUNCTION server_insert (ip inet, port uint2)
RETURNS boolean
LANGUAGE plpgsql
AS $insert_server_func$
DECLARE
    id int;
BEGIN
    INSERT INTO server_details (ADDRESS, PORT) VALUES (
        ip, port
    ) RETURNING SERVER_ID INTO id;

    IF NOT FOUND THEN
        RETURN FALSE;
    END IF;
    
    INSERT INTO server_active (SERVER_ID, ACTIVE) VALUES ( 
        id,
        TRUE
    );

    RETURN FOUND;
END;
$insert_server_func$;

CREATE OR REPLACE FUNCTION server_delete (ip inet, port uint2)
RETURNS boolean
LANGUAGE plpgsql
AS $delete_server_func$
DECLARE
BEGIN
    DELETE FROM server_details
    WHERE ADDRESS = ip AND PORT = port;

    RETURN FOUND;
END;
$delete_server_func$;

CREATE OR REPLACE FUNCTION server_deactivate (ip inet, port uint2)
RETURNS boolean
LANGUAGE plpgsql
AS $deactivate_server_func$
DECLARE
    id int;
BEGIN
    SELECT server_id
    INTO id
    FROM server_details
    WHERE ADDRESS = ip AND PORT = port;

    IF NOT FOUND THEN
        RETURN FALSE;
    END IF;

    UPDATE server_active
    SET ACTIVE = FALSE
    WHERE SERVER_ID = id;

    RETURN FOUND;
END;
$deactivate_server_func$;

CREATE OR REPLACE FUNCTION server_isactive (ip inet, port uint2)
RETURNS boolean
LANGUAGE plpgsql
AS $isactive_server_func$
DECLARE
    is_active boolean;
BEGIN
    is_active := FALSE;

    SELECT active
    INTO is_active
    FROM server_details
    INNER JOIN server_active 
    ON server_details.server_id = server_active.server_id;

    RETURN is_active;
END;
$isactive_server_func$;

CREATE OR REPLACE FUNCTION server_selectactive ()
RETURNS TABLE (address inet, port uint2)
LANGUAGE plpgsql 
AS $selectactive_server_func$
#variable_conflict use_column
DECLARE
BEGIN
    RETURN QUERY
        SELECT address, port
        FROM server_details
        INNER JOIN server_active 
        ON server_details.server_id = server_active.server_id
        WHERE active = true;
END;
$selectactive_server_func$;

CREATE OR REPLACE FUNCTION channel_insert (ip inet, port uint2, c_id bigint, g_id bigint)
RETURNS boolean
LANGUAGE plpgsql
AS $insert_server_func$
DECLARE
    id int;
BEGIN
    SELECT server_id
    INTO id
    FROM server_details
    WHERE ADDRESS = ip AND PORT = port;

    IF NOT FOUND THEN
        RETURN FALSE;
    END IF;
    
    INSERT INTO discord_channel (CHANNEL_ID, GUILD_ID) VALUES( 
        c_id,
        g_id
    );

    IF NOT FOUND THEN
        RETURN FALSE;
    END IF;

    INSERT INTO channel_server (SERVER_ID, CHANNEL_ID) VALUES( 
        id,
        c_id
    );

    RETURN TRUE;
END;
$insert_server_func$;
