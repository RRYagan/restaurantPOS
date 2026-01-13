	
	PRAGMA foreign_keys = ON;
	PRAGMA table_info(product);

	-- =============================================================================
	-- 0. METADATA
	-- =============================================================================
	CREATE TABLE IF NOT EXISTS schema_info (
	    key TEXT PRIMARY KEY,
	    value TEXT
	);

	CREATE TABLE IF NOT EXISTS schema_migration (
	    version TEXT PRIMARY KEY,
	    applied_at DATETIME NOT NULL
	);



	-- =============================================================================
	-- 2. USERS & AUTH
	-- =============================================================================
	CREATE TABLE IF NOT EXISTS tbl_user (
	    id TEXT PRIMARY KEY NOT NULL,
	    staff_id_number TEXT UNIQUE NOT NULL,
	    first_name TEXT NOT NULL,
	    last_name TEXT NOT NULL,
	    user_role TEXT CHECK(user_role IN ('Admin','Manager','Waiter','Cashier')) NOT NULL,
	    login_username TEXT UNIQUE NOT NULL,
	    password_hash TEXT NOT NULL,
	    is_active_status TEXT DEFAULT 'Y',
	    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
	);

	CREATE TABLE IF NOT EXISTS user_session (
	    id TEXT PRIMARY KEY NOT NULL,
	    user_id INTEGER NOT NULL,
	    session_token TEXT UNIQUE NOT NULL,
	    terminal_name TEXT,
	    login_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
	    expiry_timestamp DATETIME,
	    is_revoked INTEGER DEFAULT 0,
	    FOREIGN KEY (user_id) REFERENCES tbl_user(id) ON DELETE CASCADE
	);

	-- =============================================================================
	-- 3. PRODUCTS & BOM
	-- =============================================================================
	CREATE TABLE IF NOT EXISTS product (
	    id TEXT PRIMARY KEY NOT NULL,
	    inventory_product_id TEXT NOT NULL,
	    kra_unique_item_code TEXT UNIQUE NOT NULL,
	    internal_product_name TEXT NOT NULL,
	    product_category_code TEXT NOT NULL,
	    default_selling_price REAL DEFAULT 0.0,
	    tax_classification_id INTEGER,
	    quantity INTEGER NOT NULL,
	    quantity_unit_id INTEGER NOT NULL,
	    FOREIGN KEY (tax_classification_id) REFERENCES tax_classification(id),
	    FOREIGN KEY (quantity_unit_id) REFERENCES quantity_unit(id),
	    FOREIGN KEY (inventory_product_id) REFERENCES inventory(id) ON DELETE CASCADE
	);

	CREATE TABLE IF NOT EXISTS product_composition (
	    id TEXT PRIMARY KEY NOT NULL,
	    product_id TEXT NOT NULL,
	    inventory_product_id TEXT NOT NULL,
	    required_quantity REAL NOT NULL CHECK(required_quantity > 0),
	    quantity_unit_id INTEGER NOT NULL,
	    FOREIGN KEY (product_id) REFERENCES product(id) ON DELETE CASCADE,
	    FOREIGN KEY (inventory_product_id) REFERENCES inventory(id),
	    UNIQUE(product_id, inventory_product_id),
	    FOREIGN KEY (quantity_unit_id) REFERENCES quantity_unit(id)
	);

	CREATE TABLE IF NOT EXISTS inventory (
	    id TEXT PRIMARY KEY NOT NULL,
	    name TEXT NOT NULL,
	    packages_available REAL NOT NULL DEFAULT 0,
	    packaging_unit_id INTEGER NOT NULL,
	    quantity_av ailable REAL NOT NULL DEFAULT 0.0,
	    quantity_unit_id INTEGER NOT NULL,
	    last_updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,  
	    FOREIGN KEY (quantity_unit_id) REFERENCES quantity_unit(id),
	     FOREIGN KEY (packaging_unit_id) REFERENCES packaging_unit(id)
	);

	CREATE TABLE IF NOT EXISTS inventory_movement_log (
	    id TEXT PRIMARY KEY NOT NULL,
	    user_id TEXT,
	    product_id TEXT NOT NULL,
	    movement_quantity REAL NOT NULL,
	    movement_category_id INTEGER NOT NULL,
	    related_transaction_id INTEGER,
	    logged_at_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
	    FOREIGN KEY (user_id) REFERENCES tbl_user(id),
	    FOREIGN KEY (product_id) REFERENCES product(id),
	    FOREIGN KEY (movement_category_id) REFERENCES stock_movement_category(id)
	);
	-- =============================================================================
	-- 4. LEDGERS & INVENTORY
	-- =============================================================================
	CREATE TABLE IF NOT EXISTS sales_revenue_ledger (
	    id TEXT PRIMARY KEY NOT NULL,
	    user_id TEXT,
	    sale_transaction_date TEXT NOT NULL,
	    gross_total_amount REAL NOT NULL,
	    total_tax_amount REAL NOT NULL,
	    net_revenue_amount REAL NOT NULL,
	    kra_receipt_number TEXT UNIQUE,
	    kra_digital_signature TEXT,
	    payment_method_id INTEGER NOT NULL,
	    recorded_at_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
	    FOREIGN KEY (user_id) REFERENCES tbl_user(id)
	    FOREIGN KEY (payment_method_id) REFERENCES payment_method(id)
	    
	);

	CREATE TABLE IF NOT EXISTS purchase_expense_ledger (
	    id TEXT PRIMARY KEY NOT NULL,
	    user_id TEXT,
	    purchase_date TEXT NOT NULL,
	    supplier_pin_number TEXT NOT NULL,
	    total_invoice_amount REAL NOT NULL,
	    input_tax_amount REAL NOT NULL,
	    net_purchase_cost REAL NOT NULL,
	    FOREIGN KEY (user_id) REFERENCES tbl_user(id)
	);


	-- =============================================================================
	-- 5. STOCK BALANCE TRIGGER (DIRECTION-AWARE)
	-- =============================================================================
	CREATE TRIGGER IF NOT EXISTS trg_inventory_balance
	AFTER INSERT ON inventory_movement_log
	BEGIN
	    INSERT INTO inventory_stock(product_id, current_quantity)
	    VALUES (
		NEW.product_id,
		CASE
		    WHEN (
		        SELECT flow_direction
		        FROM stock_movement_category
		        WHERE id = NEW.movement_category_id
		    ) = 'INCOMING'
		    THEN NEW.movement_quantity
		    ELSE -NEW.movement_quantity
		END
	    )
	    ON CONFLICT(product_id) DO UPDATE SET
		current_quantity = current_quantity +
		CASE
		    WHEN (
		        SELECT flow_direction
		        FROM stock_movement_category
		        WHERE id = NEW.movement_category_id
		    ) = 'INCOMING'
		    THEN NEW.movement_quantity
		    ELSE -NEW.movement_quantity
		END,
		last_updated_at = CURRENT_TIMESTAMP;
	END;

	-- =============================================================================
	-- 1. LOOKUP TABLES
	-- =============================================================================
	CREATE TABLE IF NOT EXISTS tax_classification (
	    id INTEGER PRIMARY KEY AUTOINCREMENT,
	    tax_type_code TEXT UNIQUE NOT NULL,
	    tax_classification_name TEXT NOT NULL,
	    tax_percentage_rate REAL NOT NULL DEFAULT 0.0,
	    is_active_flag TEXT DEFAULT 'Y'
	);


	CREATE TABLE IF NOT EXISTS product_type (
	    id INTEGER PRIMARY KEY AUTOINCREMENT,
	    type_code INTEGER NPT NULL,
	    type_code_name TEXT NOT NULL
	);
	CREATE TABLE IF NOT EXISTS stock_movement_category (
	    id INTEGER PRIMARY KEY AUTOINCREMENT,
	    movement_type_code TEXT UNIQUE NOT NULL,
	    movement_description TEXT NOT NULL,
	    flow_direction TEXT CHECK(flow_direction IN ('INCOMING','OUTGOING')) NOT NULL
	);

	CREATE TABLE IF NOT EXISTS purchase_receipt_type (
	    id INTEGER PRIMARY KEY AUTOINCREMENT,
	    receipt_type_code CHARACTER NOT NULL,
	    receipt_type_code_name TEXT NOT NULL
	);

	CREATE TABLE IF NOT EXISTS payment_method (
	    id INTEGER PRIMARY KEY AUTOINCREMENT,
	    payment_method_code INTEGER NOT NULL,
	    payment_method_code_name TEXT NOT NULL
	);

	CREATE TABLE IF NOT EXISTS sales_receipt_type (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	receipt_type_code CHARACTER(1) NOT NULL,
    	receipt_type_name TEXT NOT NULL
	);

	INSERT OR IGNORE INTO sales_receipt_type (id, receipt_type_code, receipt_type_name) VALUES
	(NULL, 'S', 'Sale'),
	(NULL, 'R', 'Credit Note');

	CREATE TABLE IF NOT EXISTS transaction_type (
    	id INTEGER PRIMARY KEY AUTOINCREMENT,
    	transaction_type_code CHARACTER(1) NOT NULL,
    	transaction_type_name TEXT NOT NULL
	);

	INSERT OR IGNORE INTO transaction_type (id, transaction_type_code, transaction_type_name) VALUES
	(NULL, 'C', 'Copy'),
	(NULL, 'N', 'Normal'),
	(NULL, 'P', 'Proforma'),
	(NULL, 'T', 'Training');

	CREATE TABLE IF NOT EXISTS quantity_unit (
	    id INTEGER PRIMARY KEY AUTOINCREMENT,
	    quantity_unit_code TEXT NOT NULL,
	    quantity_unit_code_name TEXT NOT NULL
	);

	CREATE TABLE IF NOT EXISTS packaging_unit (
	    id INTEGER PRIMARY KEY AUTOINCREMENT,
	    packaging_unit_code TEXT NOT NULL,
	    packaging_unit_code_name TEXT NOT NULL
	);

	-- =============================================================================
	-- 6. SEED LOOKUPS
	-- =============================================================================

	INSERT OR IGNORE INTO packaging_unit VALUES
	(NULL, 'AM', 'Ampoule'),
	(NULL, 'BA', 'Barrel'),
	(NULL, 'BC', 'Bottlecrate'),
	(NULL, 'BE', 'Bundle'),
	(NULL, 'BF', 'Balloon, non-protected'),
	(NULL, 'BG', 'Bag'),
	(NULL, 'BJ', 'Bucket'),
	(NULL, 'BK', 'Basket'),
	(NULL, 'BL', 'Bale'),
	(NULL, 'BQ', 'Bottle, protected cylindrical'),
	(NULL, 'BR', 'Bar'),
	(NULL, 'BV', 'Bottle, bulbous'),
	(NULL, 'BZ', 'Bag'),
	(NULL, 'CA', 'Can'),
	(NULL, 'CH', 'Chest'),
	(NULL, 'CJ', 'Coffin'),
	(NULL, 'CL', 'Coil'),
	(NULL, 'CR', 'Wooden Box, Wooden Case'),
	(NULL, 'CS', 'Cassette'),
	(NULL, 'CT', 'Carton'),
	(NULL, 'CTN', 'Container'),
	(NULL, 'CY', 'Cylinder'),
	(NULL, 'DR', 'Drum'),
	(NULL, 'GT', 'Extra Countable Item'),
	(NULL, 'HH', 'Hand Baggage'),
	(NULL, 'IZ', 'Ingots'),
	(NULL, 'JR', 'Jar'),
	(NULL, 'JU', 'Jug'),
	(NULL, 'JY', 'Jerry CAN Cylindrical'),
	(NULL, 'KZ', 'Canester'),
	(NULL, 'LZ', 'Logs, in bundle/bunch/truss'),
	(NULL, 'NT', 'Net'),
	(NULL, 'OU', 'Non-Exterior Packaging Unit'),
	(NULL, 'PD', 'Poddon'),
	(NULL, 'PG', 'Plate'),
	(NULL, 'PI', 'Pipe'),
	(NULL, 'PO', 'Pilot'),
	(NULL, 'PU', 'Traypack'),
	(NULL, 'RL', 'Reel'),
	(NULL, 'RO', 'Roll'),
	(NULL, 'RZ', 'Rods, in bundle/bunch/truss'),
	(NULL, 'SK', 'Skeletoncase'),
	(NULL, 'TY', 'Tank, cylindrical'),
	(NULL, 'VG', 'Bulk, gas'),
	(NULL, 'VL', 'Bulk, liquid'),
	(NULL, 'VO', 'Bulk, solid, large particles'),
	(NULL, 'VQ', 'Bulk, solid'),
	(NULL, 'VR', 'Bulk, solid, granular particles'),
	(NULL, 'VT', 'Extra Bulk Item'),
	(NULL, 'VY', 'Bulk, fine particles'),
	(NULL, 'ML', 'Millscigarette Mills'),
	(NULL, 'TN', 'TAN1TAN REFER TO 20BAGS');

	INSERT OR IGNORE INTO quantity_unit VALUES
	(NULL, '4B', 'Pair'),
	(NULL, 'AV', 'Cap'),
	(NULL, 'BA', 'Barrel'),
	(NULL, 'BE', 'bundle'),
	(NULL, 'BG', 'bag'),
	(NULL, 'BL', 'block'),
	(NULL, 'BLL', 'BLL Barrel'),  --BLL Barrel (petroleum) (158,987 dm3)
	(NULL, 'BX', 'box'),
	(NULL, 'CA', 'Can'),
	(NULL, 'CEL', 'Cell'),
	(NULL, 'CMT', 'centimetre'),
	(NULL, 'CR', 'CARAT'),
	(NULL, 'DR', 'Drum'),
	(NULL, 'DZ', 'Dozen'),
	(NULL, 'GLL', 'Gallon'),
	(NULL, 'GRM', 'Gram'),
	(NULL, 'GRO', 'Gross'),
	(NULL, 'KG', 'Kilogram'),
	(NULL, 'KTM', 'kilometre'),
	(NULL, 'KWT', 'kilowatt'),
	(NULL, 'L', 'Litre'),
	(NULL, 'LBR', 'pound'),
	(NULL, 'LK', 'link'),
	(NULL, 'LTR', 'Litre'),
	(NULL, 'M', 'Metre'),
	(NULL, 'M2', 'Square Metre'),
	(NULL, 'M3', 'Cubic Metre'),
	(NULL, 'MGM', 'milligram'),
	(NULL, 'MTR', 'metre'),
	(NULL, 'MWT', 'megawatt hour'),
	(NULL, 'NO', 'Number'),
	(NULL, 'NX', 'part per thousand'),
	(NULL, 'PA', 'packet'),
	(NULL, 'PG', 'plate'),
	(NULL, 'PR', 'pair'),
	(NULL, 'RL', 'reel'),
	(NULL, 'RO', 'roll'),
	(NULL, 'SET', 'set'),
	(NULL, 'ST', 'sheet'),
	(NULL, 'TNE', 'tonne'),
	(NULL, 'TU', 'tube'),
	(NULL, 'U', 'Pieces/item'),
	(NULL, 'YRD', 'yard');


	

	INSERT OR IGNORE INTO payment_method VALUES
	(NULL, '01', 'CASH'),
	(NULL, '02', 'CREDIT'),
	(NULL, '03', 'CASH/CREDIT'),
	(NULL, '04', 'BANK CHECK'),
	(NULL, '05', 'DEBIT&CREDIT CARD'),
	(NULL, '06', 'MOBILE MONEY'),
	(NULL, '07', 'OTHER');


	INSERT OR IGNORE INTO purchase_receipt_type VALUES
	(NULL, 'P', 'Purchase'),
	(NULL, 'R', 'Credit Note after Purchase');

	INSERT OR IGNORE INTO tax_classification VALUES
	(NULL,'A','VAT 16%',16,'Y'),
	(NULL,'B','VAT 8%',8,'Y'),
	(NULL,'C','Zero Rated',0,'Y'),
	(NULL,'E','Exempt',0,'Y');

	

	INSERT OR IGNORE INTO product_type VALUES
	(NULL,'1','Raw Material'),
	(NULL,'2','Finished Product'),
	(NULL,'3','Service');


	INSERT OR IGNORE INTO stock_movement_category VALUES
	(NULL,'01','Import','INCOMING-IMPORT'),
	(NULL,'02','Purchase','INCOMING-PURCHASE'),
	(NULL,'03','Return','INCOMING-RETURN'),
	(NULL,'04','Stock Movement','INCOMING-STOCKMOVEMENT'),
	(NULL,'05','Processing','INCOMING-PROCESSING'),
	(NULL,'06','Adjustment','INCOMING-ADJUSTMENT'),
	(NULL,'11','Sale','OUTGOING-SALE'),
	(NULL,'12','Return','OUTGOING-RETURN'),
	(NULL,'13','Stock Movement','OUTGOING-STOCKMOVEMENT'),
	(NULL,'14','Processing','OUTGOING-PROCESSING'),
	(NULL,'15','Discarding','OUTGOING-DISCARDING'),
	(NULL,'16','Adjustment','OUTGOING');


	INSERT OR REPLACE INTO schema_info (key, value)
	VALUES ('version','1.0.0'),
	       ('seeded','false');

