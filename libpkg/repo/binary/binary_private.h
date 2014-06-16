/* Copyright (c) 2014, Vsevolod Stakhov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef INIT_PRIVATE_H_
#define INIT_PRIVATE_H_

#include <sqlite3.h>

#include "pkg.h"
#include "private/pkg.h"

static const char binary_repo_initsql[] = ""
	"CREATE TABLE packages ("
	    "id INTEGER PRIMARY KEY,"
	    "origin TEXT UNIQUE,"
	    "name TEXT NOT NULL,"
	    "version TEXT NOT NULL,"
	    "comment TEXT NOT NULL,"
	    "desc TEXT NOT NULL,"
	    "osversion TEXT,"
	    "arch TEXT NOT NULL,"
	    "maintainer TEXT NOT NULL,"
	    "www TEXT,"
	    "prefix TEXT NOT NULL,"
	    "pkgsize INTEGER NOT NULL,"
	    "flatsize INTEGER NOT NULL,"
	    "licenselogic INTEGER NOT NULL,"
	    "cksum TEXT NOT NULL,"
	    /* relative path to the package in the repository */
	    "path TEXT NOT NULL,"
	    "pkg_format_version INTEGER,"
	    "manifestdigest TEXT NULL,"
	    "olddigest TEXT NULL"
	");"
	"CREATE TABLE deps ("
	    "origin TEXT,"
	    "name TEXT,"
	    "version TEXT,"
	    "package_id INTEGER REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "UNIQUE(package_id, origin)"
	");"
	"CREATE TABLE categories ("
	    "id INTEGER PRIMARY KEY, "
	    "name TEXT NOT NULL UNIQUE "
	");"
	"CREATE TABLE pkg_categories ("
	    "package_id INTEGER REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "category_id INTEGER REFERENCES categories(id)"
	    "  ON DELETE RESTRICT ON UPDATE RESTRICT,"
	    "UNIQUE(package_id, category_id)"
	");"
	"CREATE TABLE licenses ("
	    "id INTEGER PRIMARY KEY,"
	    "name TEXT NOT NULL UNIQUE"
	");"
	"CREATE TABLE pkg_licenses ("
	    "package_id INTEGER REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "license_id INTEGER REFERENCES licenses(id)"
	    "  ON DELETE RESTRICT ON UPDATE RESTRICT,"
	    "UNIQUE(package_id, license_id)"
	");"
	"CREATE TABLE option ("
		"option_id INTEGER PRIMARY KEY,"
		"option TEXT NOT NULL UNIQUE"
	");"
	"CREATE TABLE option_desc ("
		"option_desc_id INTEGER PRIMARY KEY,"
		"option_desc TEXT NOT NULL UNIQUE"
	");"
	"CREATE TABLE pkg_option ("
		"package_id INTEGER NOT NULL REFERENCES packages(id) "
			"ON DELETE CASCADE ON UPDATE CASCADE,"
		"option_id INTEGER NOT NULL REFERENCES option(option_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"value TEXT NOT NULL,"
		"PRIMARY KEY(package_id, option_id)"
	");"
	"CREATE TABLE pkg_option_desc ("
		"package_id INTEGER NOT NULL REFERENCES packages(id) "
			"ON DELETE CASCADE ON UPDATE CASCADE,"
		"option_id INTEGER NOT NULL REFERENCES option(option_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"option_desc_id INTEGER NOT NULL "
			"REFERENCES option_desc(option_desc_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"PRIMARY KEY(package_id, option_id)"
	");"
	"CREATE TABLE pkg_option_default ("
		"package_id INTEGER NOT NULL REFERENCES packages(id) "
			"ON DELETE CASCADE ON UPDATE CASCADE,"
		"option_id INTEGER NOT NULL REFERENCES option(option_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"default_value TEXT NOT NULL,"
		"PRIMARY KEY(package_id, option_id)"
	");"
	"CREATE TABLE shlibs ("
	    "id INTEGER PRIMARY KEY,"
	    "name TEXT NOT NULL UNIQUE "
	");"
	"CREATE TABLE pkg_shlibs_required ("
	    "package_id INTEGER NOT NULL REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "shlib_id INTEGER NOT NULL REFERENCES shlibs(id)"
	    "  ON DELETE RESTRICT ON UPDATE RESTRICT,"
	    "UNIQUE(package_id, shlib_id)"
	");"
	"CREATE TABLE pkg_shlibs_provided ("
	    "package_id INTEGER NOT NULL REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "shlib_id INTEGER NOT NULL REFERENCES shlibs(id)"
	    "  ON DELETE RESTRICT ON UPDATE RESTRICT,"
	    "UNIQUE(package_id, shlib_id)"
	");"
	"CREATE TABLE annotation ("
	    "annotation_id INTEGER PRIMARY KEY,"
	    "annotation TEXT NOT NULL UNIQUE"
	");"
	"CREATE TABLE pkg_annotation ("
	    "package_id INTERGER REFERENCES packages(id)"
	    " ON DELETE CASCADE ON UPDATE RESTRICT,"
	    "tag_id INTEGER NOT NULL REFERENCES annotation(annotation_id)"
	    " ON DELETE CASCADE ON UPDATE RESTRICT,"
	    "value_id INTEGER NOT NULL REFERENCES annotation(annotation_id)"
	    " ON DELETE CASCADE ON UPDATE RESTRICT,"
	    "UNIQUE (package_id, tag_id)"
	");"
	"CREATE TABLE pkg_conflicts ("
	    "package_id INTEGER NOT NULL REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "conflict_id INTEGER NOT NULL,"
	    "UNIQUE(package_id, conflict_id)"
	");"
	"CREATE TABLE provides("
	"    id INTEGER PRIMARY KEY,"
	"    provide TEXT NOT NULL"
	");"
	"CREATE TABLE pkg_provides ("
	    "package_id INTEGER NOT NULL REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "provide_id INTEGER NOT NULL REFERENCES provides(id)"
	    "  ON DELETE RESTRICT ON UPDATE RESTRICT,"
	    "UNIQUE(package_id, provide_id)"
	");"
	"CREATE INDEX packages_origin ON packages(origin COLLATE NOCASE);"
	"CREATE INDEX packages_name ON packages(name COLLATE NOCASE);"
	"CREATE INDEX packages_uid_nocase ON packages(name COLLATE NOCASE, origin COLLATE NOCASE);"
	"CREATE INDEX packages_version_nocase ON packages(name COLLATE NOCASE, version);"
	"CREATE INDEX packages_uid ON packages(name, origin);"
	"CREATE INDEX packages_version ON packages(name, version);"
	"CREATE UNIQUE INDEX packages_digest ON packages(manifestdigest);"
	/* FTS search table */
	"CREATE VIRTUAL TABLE pkg_search USING fts4(id, name, origin);"

	"PRAGMA user_version=%d;"
	;

struct repo_changes {
	int version;		/* The repo schema this change applies to */
	int next_version;	/* The repo schema this change creates */
	const char *message;
	const char *sql;
};

/* How to upgrade an older repo to match what the current system
   expects */
static const struct repo_changes repo_upgrades[] = {
	{2001,
	 2002,
	 "Modify shlib tracking to add \'provided\' capability",

	 "CREATE TABLE pkg_shlibs_required ("
		"package_id INTEGER NOT NULL REFERENCES packages(id)"
		" ON DELETE CASCADE ON UPDATE CASCADE,"
		"shlib_id INTEGER NOT NULL REFERENCES shlibs(id)"
		" ON DELETE RESTRICT ON UPDATE RESTRICT,"
		"UNIQUE (package_id, shlib_id)"
	 ");"
	 "CREATE TABLE pkg_shlibs_provided ("
		"package_id INTEGER NOT NULL REFERENCES packages(id)"
		" ON DELETE CASCADE ON UPDATE CASCADE,"
		"shlib_id INTEGER NOT NULL REFERENCES shlibs(id)"
		" ON DELETE RESTRICT ON UPDATE RESTRICT,"
		"UNIQUE (package_id, shlib_id)"
	 ");"
	 "INSERT INTO pkg_shlibs_required (package_id, shlib_id)"
		" SELECT package_id, shlib_id FROM pkg_shlibs;"
	 "DROP TABLE pkg_shlibs;"
	},
	{2002,
	 2003,
	 "Add abstract metadata capability",

	 "CREATE TABLE abstract ("
		"abstract_id INTEGER PRIMARY KEY,"
		"abstract TEXT NOT NULL UNIQUE"
	 ");"
	 "CREATE TABLE pkg_abstract ("
		"package_id INTERGER REFERENCES packages(id)"
		" ON DELETE CASCADE ON UPDATE RESTRICT,"
		"key_id INTEGER NOT NULL REFERENCES abstract(abstract_id)"
		" ON DELETE CASCADE ON UPDATE RESTRICT,"
		"value_id INTEGER NOT NULL REFERENCES abstract(abstract_id)"
		" ON DELETE CASCADE ON UPDATE RESTRICT"
	 ");"
	},
	{2003,
	 2004,
	"Add manifest digest field",

	"ALTER TABLE packages ADD COLUMN manifestdigest TEXT NULL;"
	"CREATE INDEX IF NOT EXISTS pkg_digest_id ON packages(origin, manifestdigest);"
	},
	{2004,
	 2005,
	 "Rename 'abstract metadata' to 'annotations'",

	 "CREATE TABLE annotation ("
	        "annotation_id INTEGER PRIMARY KEY,"
	        "annotation TEXT NOT NULL UNIQUE"
	 ");"
	 "CREATE TABLE pkg_annotation ("
	        "package_id INTEGER REFERENCES packages(id)"
	        " ON DELETE CASCADE ON UPDATE RESTRICT,"
	        "tag_id INTEGER NOT NULL REFERENCES annotation(annotation_id)"
	        " ON DELETE CASCADE ON UPDATE RESTRICT,"
	        "value_id INTEGER NOT NULL REFERENCES annotation(annotation_id)"
	        " ON DELETE CASCADE ON UPDATE RESTRICT,"
	        "UNIQUE (package_id, tag_id)"
	 ");"
	 "INSERT INTO annotation (annotation_id, annotation)"
	        " SELECT abstract_id, abstract FROM abstract;"
	 "INSERT INTO pkg_annotation (package_id,tag_id,value_id)"
	        " SELECT package_id,key_id,value_id FROM pkg_abstract;"
	 "DROP TABLE pkg_abstract;"
	 "DROP TABLE abstract;"
	},
	{2005,
	 2006,
	 "Add capability to track option descriptions and defaults",

	 "CREATE TABLE option ("
		"option_id INTEGER PRIMARY KEY,"
		"option TEXT NOT NULL UNIQUE"
	 ");"
	 "CREATE TABLE option_desc ("
		"option_desc_id INTEGER PRIMARY KEY,"
		"option_desc TEXT NOT NULL UNIQUE"
	 ");"
	 "CREATE TABLE pkg_option ("
		"package_id INTEGER NOT NULL REFERENCES packages(id) "
			"ON DELETE CASCADE ON UPDATE CASCADE,"
		"option_id INTEGER NOT NULL REFERENCES option(option_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"value TEXT NOT NULL,"
		"PRIMARY KEY(package_id, option_id)"
	 ");"
	 "CREATE TABLE pkg_option_desc ("
		"package_id INTEGER NOT NULL REFERENCES packages(id) "
			"ON DELETE CASCADE ON UPDATE CASCADE,"
		"option_id INTEGER NOT NULL REFERENCES option(option_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"option_desc_id INTEGER NOT NULL "
			"REFERENCES option_desc(option_desc_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"PRIMARY KEY(package_id, option_id)"
	 ");"
	 "CREATE TABLE pkg_option_default ("
		"package_id INTEGER NOT NULL REFERENCES packages(id) "
			"ON DELETE CASCADE ON UPDATE CASCADE,"
		"option_id INTEGER NOT NULL REFERENCES option(option_id) "
			"ON DELETE RESTRICT ON UPDATE CASCADE,"
		"default_value TEXT NOT NULL,"
		"PRIMARY KEY(package_id, option_id)"
	 ");"
	 "INSERT INTO option (option) "
		"SELECT DISTINCT option FROM options;"
	 "INSERT INTO pkg_option(package_id, option_id, value) "
		"SELECT package_id, option_id, value "
		"FROM options oo JOIN option o "
			"ON (oo.option = o.option);"
	 "DROP TABLE options;",
	},
	{2006,
	 2007,
	 "Add conflicts and provides",

	"CREATE TABLE pkg_conflicts ("
	    "package_id INTEGER NOT NULL REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "conflict_id INTEGER NOT NULL,"
	    "UNIQUE(package_id, conflict_id)"
	");"
	"CREATE TABLE provides("
	"    id INTEGER PRIMARY KEY,"
	"    provide TEXT NOT NULL"
	");"
	"CREATE TABLE pkg_provides ("
	    "package_id INTEGER NOT NULL REFERENCES packages(id)"
	    "  ON DELETE CASCADE ON UPDATE CASCADE,"
	    "provide_id INTEGER NOT NULL REFERENCES provides(id)"
	    "  ON DELETE RESTRICT ON UPDATE RESTRICT,"
	    "UNIQUE(package_id, provide_id)"
	");"
	},
	{2007,
	 2008,
	 "Add FTS index",

	 "CREATE VIRTUAL TABLE pkg_search USING fts4(id, name, origin);"
	 "INSERT INTO pkg_search SELECT id, name || '-' || version, origin FROM packages;"
	 "CREATE INDEX packages_origin ON packages(origin COLLATE NOCASE);"
	 "CREATE INDEX packages_name ON packages(name COLLATE NOCASE);"
	},
	{2008,
	 2009,
	 "Optimize indicies",

	 "CREATE INDEX IF NOT EXISTS packages_uid_nocase ON packages(name COLLATE NOCASE, origin COLLATE NOCASE);"
	 "CREATE INDEX IF NOT EXISTS packages_version_nocase ON packages(name COLLATE NOCASE, version);"
	 "CREATE INDEX IF NOT EXISTS packages_uid ON packages(name, origin);"
	 "CREATE INDEX IF NOT EXISTS packages_version ON packages(name, version);"
	 "CREATE UNIQUE INDEX IF NOT EXISTS packages_digest ON packages(manifestdigest);"
	},
	{2009,
	 2010,
	 "Add legacy digest field",

	 "ALTER TABLE packages ADD COLUMN olddigest TEXT NULL;"
	 "UPDATE packages SET olddigest=manifestdigest WHERE olddigest=NULL;"
	},
	/* Mark the end of the array */
	{ -1, -1, NULL, NULL, }

};

/* How to downgrade a newer repo to match what the current system
   expects */
static const struct repo_changes repo_downgrades[] = {
	{2010,
	 2009,
	 "Drop olddigest field",

	 "ALTER TABLE packages REMOVE COLUMN olddigest;"
	},
	{2009,
	 2008,
	 "Drop indicies",

	 "DROP INDEX packages_uid_nocase;"
	 "DROP INDEX packages_version_nocase;"
	 "DROP INDEX packages_uid;"
	 "DROP INDEX packages_version;"
	 "DROP INDEX packages_digest;"
	},
	{2008,
	 2007,
	 "Drop FTS index",

	 "DROP TABLE pkg_search;"
	},
	{2007,
	 2006,
	 "Revert conflicts and provides creation",

	 "DROP TABLE pkg_provides;"
	 "DROP TABLE provides;"
	 "DROP TABLE conflicts;"
	},
	{2006,
	 2005,
	 "Revert addition of extra options related data",

	 "CREATE TABLE options ("
		"package_id INTEGER REFERENCES packages(id) "
			"ON DELETE CASCADE ON UPDATE CASCADE,"
		"option TEXT,"
		"value TEXT,"
		"PRIMARY KEY(package_id,option)"
	 ");"
	 "INSERT INTO options (package_id, option, value) "
		 "SELECT package_id, option, value "
		"FROM pkg_option JOIN option USING(option_id);"
	 "DROP TABLE pkg_option;"
	 "DROP TABLE pkg_option_default;"
	 "DROP TABLE option;"
	 "DROP TABLE pkg_option_desc;"
	 "DROP TABLE option_desc;",
	},
	{2005,
	 2004,
	 "Revert rename of 'abstract metadata' to 'annotations'",

	 "CREATE TABLE abstract ("
	        "abstract_id INTEGER PRIMARY KEY,"
	        "abstract TEXT NOT NULL UNIQUE"
	 ");"
	 "CREATE TABLE pkg_abstract ("
	        "package_id INTEGER REFERENCES packages(id)"
	        " ON DELETE CASCADE ON UPDATE RESTRICT,"
	        "key_id INTEGER NOT NULL REFERENCES abstract(abstract_id)"
	        " ON DELETE CASCADE ON UPDATE RESTRICT,"
	        "value_id INTEGER NOT NULL REFERENCES abstract(abstract_id)"
	        " ON DELETE CASCADE ON UPDATE RESTRICT"
	 ");"
	 "INSERT INTO abstract (abstract_id, abstract)"
	        " SELECT annotation_id, annotation FROM annotation;"
	 "INSERT INTO pkg_abstract (package_id,key_id,value_id)"
	        " SELECT package_id,tag_id,value_id FROM pkg_annotation;"
	 "DROP TABLE pkg_annotation;"
	 "DROP TABLE annotation;"
	},
	{2004,
	 2003,
	 "Drop manifest digest index",

	 "DROP INDEX pkg_digest_id;"
	},
	{2003,
	 2002,
	 "Drop abstract metadata",

	 "DROP TABLE pkg_abstract;"
	 "DROP TABLE abstract;"
	},
	{2002,
	 2001,
	 "Drop \'shlibs provided\' but retain \'shlibs required\'",

	 "CREATE TABLE pkg_shlibs_required ("
		"package_id INTEGER NOT NULL REFERENCES packages(id)"
		" ON DELETE CASCADE ON UPDATE CASCADE,"
		"shlib_id INTEGER NOT NULL REFERENCES shlibs(id)"
		" ON DELETE RESTRICT ON UPDATE RESTRICT,"
		"UNIQUE (package_id, shlib_id)"
	 ");"
	 "CREATE TABLE pkg_shlibs ("
		"package_id INTEGER REFERENCES packages(id)"
		" ON DELETE CASCADE ON UPDATE CASCADE,"
		"shlib_id INTEGER REFERENCES shlibs(id)"
		" ON DELETE RESTRICT ON UPDATE RESTRICT,"
		"PRIMARY KEY (package_id, shlib_id)"
	 ");"
	 "INSERT INTO pkg_shlibs (package_id, shlib_id)"
		" SELECT package_id, shlib_id FROM pkg_shlibs_required;"
	 "DELETE FROM shlibs WHERE id NOT IN"
		" (SELECT shlib_id FROM pkg_shlibs);"
	 "DROP TABLE pkg_shlibs_provided;"
	 "DROP TABLE pkg_shlibs_required;"
	},


	/* Mark the end of the array */
	{ -1, -1, NULL, NULL, }

};

/* The package repo schema major revision */
#define REPO_SCHEMA_MAJOR 2

/* The package repo schema minor revision.
   Minor schema changes don't prevent older pkgng
   versions accessing the repo. */
#define REPO_SCHEMA_MINOR 10

/* REPO_SCHEMA_VERSION=2007 */
#define REPO_SCHEMA_VERSION (REPO_SCHEMA_MAJOR * 1000 + REPO_SCHEMA_MINOR)

typedef enum _sql_prstmt_index {
	PKG = 0,
	DEPS,
	CAT1,
	CAT2,
	LIC1,
	LIC2,
	OPT1,
	OPT2,
	SHLIB1,
	SHLIB_REQD,
	SHLIB_PROV,
	ANNOTATE1,
	ANNOTATE2,
	EXISTS,
	VERSION,
	DELETE,
	FTS_APPEND,
	PRSTMT_LAST,
} sql_prstmt_index;

static sql_prstmt sql_prepared_statements[PRSTMT_LAST] = {
	[PKG] = {
		NULL,
		"INSERT INTO packages ("
		"origin, name, version, comment, desc, arch, maintainer, www, "
		"prefix, pkgsize, flatsize, licenselogic, cksum, path, manifestdigest, olddigest"
		")"
		"VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16)",
		"TTTTTTTTTIIITTTT",
	},
	[DEPS] = {
		NULL,
		"INSERT INTO deps (origin, name, version, package_id) "
		"VALUES (?1, ?2, ?3, ?4)",
		"TTTI",
	},
	[CAT1] = {
		NULL,
		"INSERT OR IGNORE INTO categories(name) VALUES(?1)",
		"T",
	},
	[CAT2] = {
		NULL,
		"INSERT OR ROLLBACK INTO pkg_categories(package_id, category_id) "
		"VALUES (?1, (SELECT id FROM categories WHERE name = ?2))",
		"IT",
	},
	[LIC1] = {
		NULL,
		"INSERT OR IGNORE INTO licenses(name) VALUES(?1)",
		"T",
	},
	[LIC2] = {
		NULL,
		"INSERT OR ROLLBACK INTO pkg_licenses(package_id, license_id) "
		"VALUES (?1, (SELECT id FROM licenses WHERE name = ?2))",
		"IT",
	},
	[OPT1] = {
		NULL,
		"INSERT OR IGNORE INTO option(option) "
		"VALUES (?1)",
		"T",
	},
	[OPT2] = {
		NULL,
		"INSERT OR ROLLBACK INTO pkg_option (option_id, value, package_id) "
		"VALUES (( SELECT option_id FROM option WHERE option = ?1), ?2, ?3)",
		"TTI",
	},
	[SHLIB1] = {
		NULL,
		"INSERT OR IGNORE INTO shlibs(name) VALUES(?1)",
		"T",
	},
	[SHLIB_REQD] = {
		NULL,
		"INSERT OR ROLLBACK INTO pkg_shlibs_required(package_id, shlib_id) "
		"VALUES (?1, (SELECT id FROM shlibs WHERE name = ?2))",
		"IT",
	},
	[SHLIB_PROV] = {
		NULL,
		"INSERT OR ROLLBACK INTO pkg_shlibs_provided(package_id, shlib_id) "
		"VALUES (?1, (SELECT id FROM shlibs WHERE name = ?2))",
		"IT",
	},
	[EXISTS] = {
		NULL,
		"SELECT count(*) FROM packages WHERE cksum=?1",
		"T",
	},
	[ANNOTATE1] = {
		NULL,
		"INSERT OR IGNORE INTO annotation(annotation) "
		"VALUES (?1)",
		"T",
	},
	[ANNOTATE2] = {
		NULL,
		"INSERT OR ROLLBACK INTO pkg_annotation(package_id, tag_id, value_id) "
		"VALUES (?1,"
		" (SELECT annotation_id FROM annotation WHERE annotation=?2),"
		" (SELECT annotation_id FROM annotation WHERE annotation=?3))",
		"ITT",
	},
	[VERSION] = {
		NULL,
		"SELECT version FROM packages WHERE origin=?1",
		"T",
	},
	[DELETE] = {
		NULL,
		"DELETE FROM packages WHERE origin=?1;"
		"DELETE FROM pkg_search WHERE origin=?1;",
		"TT",
	},
	[FTS_APPEND] = {
		NULL,
		"INSERT OR ROLLBACK INTO pkg_search(id, name, origin) "
		"VALUES (?1, ?2 || '-' || ?3, ?4);",
		"ITTT"
	}
	/* PRSTMT_LAST */
};

int pkg_repo_binary_run_prstatement(sql_prstmt_index s, ...);

#endif /* INIT_PRIVATE_H_ */
