-- ===========================================
-- Initial Database Schema for RestaurantPOS
-- Version: 000_init
-- ===========================================
CREATE TABLE IF NOT EXISTS schema_info (
    key TEXT PRIMARY KEY,
    value TEXT
);

-- ==============================
-- SCHEMA MIGRATIONS TRACKING
-- ==============================
CREATE TABLE IF NOT EXISTS schema_migrations (
    version TEXT PRIMARY KEY,
    checksum TEXT NOT NULL,
    applied_at DATETIME NOT NULL
);

CREATE TABLE IF NOT EXISTS migration_lock (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    locked INTEGER NOT NULL DEFAULT 0
);

-- ==============================
-- EXPENSES (Net Profit Calculation)
-- ==============================
CREATE TABLE IF NOT EXISTS expenses (
    id TEXT PRIMARY KEY,
    category_id INTEGER,
    amount_cents INTEGER,
    description TEXT,
    expense_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    user_id TEXT,
    FOREIGN KEY(category_id) REFERENCES expense_categories(id)
);

-- ==============================
-- SUPPLIER PURCHASES (Track COGS)
-- ==============================
CREATE TABLE IF NOT EXISTS purchases (
    id TEXT PRIMARY KEY,
    supplier_nm TEXT,
    total_inc_tax_cents INTEGER,
    tax_amt_cents INTEGER,
    status TEXT,             -- PAID, PENDING, CREDIT
    purchase_date DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ==============================
-- PURCHASE ITEMS (Ingredient Level Costing)
-- ==============================
CREATE TABLE IF NOT EXISTS purchase_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    purchase_id TEXT,
    item_nm TEXT,
    qty REAL,
    unit_cost_cents INTEGER,
    FOREIGN KEY(purchase_id) REFERENCES purchases(id)
);

-- ==============================
-- WASTE LOGS (Loss Analytics)
-- ==============================
CREATE TABLE IF NOT EXISTS waste_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id TEXT,
    qty REAL,
    estimated_loss_cents INTEGER,
    reason TEXT,             -- Expired, Dropped, Burnt
    occured_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(product_id) REFERENCES products(id)
);

-- ==============================
-- PRODUCTS (VSCU Aligned)
-- ==============================
CREATE TABLE IF NOT EXISTS products (
    id TEXT PRIMARY KEY,        -- Internal UUID
    item_nm TEXT NOT NULL,      -- Item Name
    item_cd TEXT UNIQUE,        -- KRA Code
    item_cls_cd TEXT,           -- 10-digit class code
    item_ty_cd TEXT,            -- 1:Raw, 2:Finished, 3:Service
    tax_ty_cd TEXT DEFAULT 'B', -- Tax Type: A,B,C,D,E
    pkg_unit_cd TEXT,           -- Packaging Unit
    qty_unit_cd TEXT,           -- Quantity Unit
    base_price_cents INTEGER,
    is_available BOOLEAN DEFAULT 1
);

-- ==============================
-- ORDERS (Fiscal Header)
-- ==============================
CREATE TABLE IF NOT EXISTS orders (
    id TEXT PRIMARY KEY,        -- Internal UUID
    table_number INTEGER,
    waiter_id TEXT,
    status TEXT,                -- OPEN, PAID, VOID
    tot_taxbl_amt_cents INTEGER,
    tot_tax_amt_cents INTEGER,
    tot_amt_cents INTEGER,
    sdc_receipt_no TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME
);

-- ==============================
-- ORDER ITEMS (Fiscal Lines)
-- ==============================
CREATE TABLE IF NOT EXISTS order_items (
    id TEXT PRIMARY KEY,
    order_id TEXT,
    product_id TEXT,
    name TEXT,
    quantity REAL,
    unit_price_cents INTEGER,
    taxbl_amt_cents INTEGER,
    tax_amt_cents INTEGER,
    tax_ty_cd TEXT,              -- Snapshot of tax type at sale
    FOREIGN KEY(order_id) REFERENCES orders(id),
    FOREIGN KEY(product_id) REFERENCES products(id)
);

-- ==============================
-- INVENTORY & MOVEMENT (Stock IO)
-- ==============================
CREATE TABLE IF NOT EXISTS inventory_stocks (
    product_id TEXT PRIMARY KEY,
    current_stock REAL DEFAULT 0,
    min_stock_level REAL,
    FOREIGN KEY(product_id) REFERENCES products(id)
);

CREATE TABLE IF NOT EXISTS inventory_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id TEXT,
    instck_ty_cd TEXT,           -- KRA Codes: 01 (Purchase), 11 (Sale), 15 (Waste)
    qty REAL,
    regr_id TEXT,                -- User who performed action
    occured_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(product_id) REFERENCES products(id)
);

-- ==============================
-- PAYMENTS (M-Pesa & Financials)
-- ==============================
CREATE TABLE IF NOT EXISTS payments (
    id TEXT PRIMARY KEY,         -- UUID or M-Pesa Ref
    order_id TEXT,
    payment_method TEXT,         -- CASH, MPESA, CARD
    amount_cents INTEGER,
    external_ref TEXT,           -- CheckoutRequestID
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(order_id) REFERENCES orders(id)
);

-- ==============================
-- ACTION LOGS (Audit Trail)
-- ==============================
CREATE TABLE IF NOT EXISTS action_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id TEXT,
    action_type TEXT,            -- VOID, DISCOUNT, LOGIN
    details TEXT,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
);
