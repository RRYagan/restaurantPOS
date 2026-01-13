PRAGMA foreign_keys = ON;
PRAGMA table_info(product);

-- =============================================================================
-- 0. METADATA
-- =============================================================================
CREATE TABLE IF NOT EXISTS schema_info (
    key TEXT PRIMARY KEY,
    value TEXT
);

CREATE TABLE IF NOT EXISTS schema_migrations (
    version TEXT PRIMARY KEY,
    applied_at DATETIME NOT NULL
);

-- =============================================================================
-- 1. LOOKUP TABLES
-- =============================================================================
CREATE TABLE IF NOT EXISTS tax_classifications (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    tax_type_code TEXT UNIQUE NOT NULL,
    tax_classification_name TEXT NOT NULL,
    tax_percentage_rate REAL NOT NULL DEFAULT 0.0,
    is_active_flag TEXT DEFAULT 'Y'
);

CREATE TABLE IF NOT EXISTS measurement_units (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    unit_code TEXT UNIQUE NOT NULL,
    unit_display_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS stock_movement_categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    movement_type_code TEXT UNIQUE NOT NULL,
    movement_description TEXT NOT NULL,
    flow_direction TEXT CHECK(flow_direction IN ('INCOMING','OUTGOING')) NOT NULL
);

-- =============================================================================
-- 2. USERS & AUTH
-- =============================================================================
CREATE TABLE IF NOT EXISTS users (
    id TEXT PRIMARY KEY,
    staff_id_number TEXT UNIQUE NOT NULL,
    full_name TEXT NOT NULL,
    user_role TEXT CHECK(user_role IN ('Admin','Manager','Waiter','Cashier')) NOT NULL,
    login_username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    is_active_status TEXT DEFAULT 'Y',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS user_sessions (
    id TEXT PRIMARY KEY,
    user_id INTEGER NOT NULL,
    session_token TEXT UNIQUE NOT NULL,
    terminal_name TEXT,
    login_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    expiry_timestamp DATETIME,
    is_revoked INTEGER DEFAULT 0,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- =============================================================================
-- 3. PRODUCTS & BOM
-- =============================================================================
CREATE TABLE IF NOT EXISTS product (
    id TEXT PRIMARY KEY,
    kra_unique_item_code TEXT UNIQUE NOT NULL,
    internal_product_name TEXT NOT NULL,
    product_category_code TEXT NOT NULL,
    default_selling_price REAL DEFAULT 0.0,
    tax_classification_id INTEGER,
    measurement_unit_id INTEGER NOT NULL,
    FOREIGN KEY (tax_classification_id) REFERENCES tax_classifications(id),
    FOREIGN KEY (measurement_unit_id) REFERENCES measurement_units(id)
);

CREATE TABLE IF NOT EXISTS product_composition (
    id TEXT PRIMARY KEY,
    main_product_item_id TEXT NOT NULL,
    ingredient_item_id TEXT NOT NULL,
    required_quantity REAL NOT NULL CHECK(required_quantity > 0),
     measurement_unit_id INTEGER NOT NULL,
    FOREIGN KEY (main_product_item_id) REFERENCES product(id) ON DELETE CASCADE,
    FOREIGN KEY (ingredient_item_id) REFERENCES product(id),
    UNIQUE(main_product_item_id, ingredient_item_id)
    FOREIGN KEY (measurement_unit_id) REFERENCES measurement_units(id)
);

-- =============================================================================
-- 4. LEDGERS & INVENTORY
-- =============================================================================
CREATE TABLE IF NOT EXISTS sales_revenue_ledger (
    id TEXT PRIMARY KEY,
    user_id TEXT,
    sale_transaction_date TEXT NOT NULL,
    gross_total_amount REAL NOT NULL,
    total_tax_amount REAL NOT NULL,
    net_revenue_amount REAL NOT NULL,
    kra_receipt_number TEXT UNIQUE,
    kra_digital_signature TEXT,
    payment_method_type TEXT NOT NULL,
    recorded_at_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS purchase_expense_ledger (
    id TEXT PRIMARY KEY,
    user_id TEXT,
    purchase_date TEXT NOT NULL,
    supplier_pin_number TEXT NOT NULL,
    total_invoice_amount REAL NOT NULL,
    input_tax_amount REAL NOT NULL,
    net_purchase_cost REAL NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS inventory_stock (
    product_id TEXT PRIMARY KEY,
    current_quantity REAL NOT NULL DEFAULT 0.0,
    minimum_threshold REAL DEFAULT 0.0,
    last_updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (product_id) REFERENCES product(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS inventory_movement_log (
    id TEXT PRIMARY KEY,
    user_id TEXT,
    product_id INTEGER NOT NULL,
    movement_quantity REAL NOT NULL,
    movement_category_id INTEGER NOT NULL,
    related_transaction_id INTEGER,
    logged_at_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (product_id) REFERENCES product(id),
    FOREIGN KEY (movement_category_id) REFERENCES stock_movement_categories(id)
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
                FROM stock_movement_categories
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
                FROM stock_movement_categories
                WHERE id = NEW.movement_category_id
            ) = 'INCOMING'
            THEN NEW.movement_quantity
            ELSE -NEW.movement_quantity
        END,
        last_updated_at = CURRENT_TIMESTAMP;
END;


-- =============================================================================
-- 6. SEED LOOKUPS
-- =============================================================================
INSERT OR IGNORE INTO tax_classifications VALUES
(NULL,'A','VAT 16%',16,'Y'),
(NULL,'B','VAT 8%',8,'Y'),
(NULL,'C','Zero Rated',0,'Y'),
(NULL,'E','Exempt',0,'Y');

INSERT OR IGNORE INTO measurement_units VALUES
(NULL,'EA','Each'),
(NULL,'KG','Kilogram'),
(NULL,'LTR','Litre'),
(NULL,'ST','Sheet');

INSERT OR IGNORE INTO stock_movement_categories VALUES
(NULL,'01','Import','INCOMING'),
(NULL,'02','Purchase','INCOMING'),
(NULL,'11','Sale','OUTGOING'),
(NULL,'15','Wastage','OUTGOING'),
(NULL,'16','Adjustment','OUTGOING');

INSERT OR REPLACE INTO schema_info (key, value)
VALUES ('version','1.0.0'),
       ('seeded','false');

