-- Schema for IRC bot.

BEGIN TRANSACTION main;

	CREATE TABLE IF NOT EXISTS Versions (
		ID INTEGER PRIMARY KEY,
		LOG TEXT
		);

COMMIT TRANSACTION main;

