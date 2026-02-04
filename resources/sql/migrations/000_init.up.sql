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
CREATE TABLE IF NOT EXISTS user_role (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    role_name TEXT NOT NULL UNIQUE,
    role_description TEXT,
    access_level INTEGER DEFAULT 1,
    max_discount_pct DECIMAL(5, 2) DEFAULT 0.00,
    can_void_order BOOLEAN DEFAULT 0,
    can_view_reports BOOLEAN DEFAULT 0,
    requires_manager_approval BOOLEAN DEFAULT 0
);
--todo: remove before deploy
INSERT OR IGNORE INTO user_role 
    (id, role_name, role_description, access_level, max_discount_pct, can_void_order, can_view_reports, requires_manager_approval) 
        VALUES
        (NULL, 'Admin', 'Full system configuration and financial oversight.', 100, 100.00, 1, 1, 0),
        (NULL, 'Manager', 'Day-to-day operations, staff management, and overrides.', 80, 25.00, 1, 1, 0),
        (NULL, 'Cashier', 'Handling payments and closing registers.', 50, 5.00, 0, 0, 1),
        (NULL, 'Waiter', 'Taking orders and serving customers.', 10, 0.00, 0, 0, 1),
        (NULL, 'Kitchen', 'Viewing and managing order tickets.', 5, 0.00, 0, 0, 0);

CREATE TABLE IF NOT EXISTS tbl_user (
    id TEXT PRIMARY KEY NOT NULL,
    staff_id_number TEXT UNIQUE NOT NULL,
    first_name TEXT NOT NULL,
    last_name TEXT NOT NULL,
    user_role_id INTEGER NOT NULL,
    login_username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    is_active_status TEXT DEFAULT 'Y',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_role_id) REFERENCES user_role(id)
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
    internal_product_name TEXT NOT NULL,
    kra_item_code TEXT NOT NULL,
    product_category_id INTEGER NOT NULL,
    product_type_id INTEGER NOT NULL,
    currency_code TEXT NOT NULL,
    country_code TEXT NOT NULL,
    default_selling_price REAL DEFAULT 0.0 NOT NULL,
    tax_classification_code TEXT NOT NULL, -- Linked to KRA codes (A, B, C, E)
    tax_amount REAL NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    -- Mandatory Foreign Keys
    FOREIGN KEY (product_type_id) REFERENCES product_type(type_code),
    FOREIGN KEY (product_category_id) REFERENCES product_category(id),
    FOREIGN KEY (tax_classification_code) REFERENCES tax_classification(tax_type_code),
    FOREIGN KEY (currency_code) REFERENCES currency(currency_code),
    FOREIGN KEY (country_code) REFERENCES country(country_code)
);

CREATE TABLE IF NOT EXISTS product_category (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    product_category_name TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP   
);
--todo: remove before deploy
INSERT OR IGNORE INTO product_category (product_category_name) VALUES
    ('Appetizers'),
    ('Main Courses'),
    ('Desserts'),
    ('Beverages'),
    ('Alcoholic Drinks'),
    ('Side Dishes'),
    ('Salads'),
    ('Soups'),
    ('Breakfast'),
    ('Specialty Coffee');


CREATE TABLE IF NOT EXISTS product_composition (
    id TEXT NOT NULL,
    product_id TEXT NOT NULL,
    inventory_id TEXT NOT NULL,
    required_quantity REAL NOT NULL, -- Amount of 'quantity_unit_id' to deduct
    quantity_unit TEXT NOT NULL,
    
    FOREIGN KEY (product_id) REFERENCES product(id) ON DELETE CASCADE,
    FOREIGN KEY (inventory_id) REFERENCES inventory(id),
    PRIMARY KEY (product_id, inventory_id)
     FOREIGN KEY (quantity_unit) REFERENCES quantity_unit(quantity_unit_code)
);

CREATE TABLE IF NOT EXISTS inventory (
    id TEXT PRIMARY KEY NOT NULL,
    name TEXT NOT NULL,
    
    -- Packaging Logic (The "Crate" or "Box")
    packaging_unit_idf TEXT NOT NULL,      -- FK to packaging_unit (e.g., 'BC' for Bottlecrate)
    total_packages_available REAL DEFAULT 0, -- e.g., 2.0 crates
    
    -- Quantity Logic (The "Bottle" or "ML")
    quantity_per_package REAL NOT NULL,    -- e.g., 24.0 (bottles per crate)
    quantity_unit_idf TEXT NOT NULL,       -- FK to quantity_unit (e.g., 'U' for Pieces)
    
    -- Total units available (Calculated: packages * quantity_per_package)
    total_quantity_available REAL DEFAULT 0, -- e.g., 48.0 bottles
    
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (packaging_unit_idf) REFERENCES packaging_unit(packaging_unit_code_name),
    FOREIGN KEY (quantity_unit_idf) REFERENCES quantity_unit(quantity_unit_code_name)
);

CREATE TABLE IF NOT EXISTS inventory_movement_log (
    id TEXT PRIMARY KEY NOT NULL,
    user_idf TEXT,
    product_idf TEXT NOT NULL,
    movement_quantity REAL NOT NULL,
    movement_category_id INTEGER NOT NULL,
    related_transaction_id INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_idf) REFERENCES tbl_user(id),
    FOREIGN KEY (product_idf) REFERENCES product(id),
    FOREIGN KEY (movement_category_id) REFERENCES stock_movement_category(id)
);

-- =============================================================================
-- 4. LEDGERS & INVENTORY
-- =============================================================================
CREATE TABLE IF NOT EXISTS customer_order (
    id TEXT PRIMARY KEY NOT NULL,
    table_number TEXT,
    waiter_id TEXT,
    order_status TEXT CHECK(order_status IN ('open', 'closed', 'voided')) DEFAULT 'open',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (waiter_id) REFERENCES tbl_user(id)
);

CREATE TABLE IF NOT EXISTS order_item (
    id TEXT PRIMARY KEY NOT NULL,
    order_id TEXT NOT NULL,
    product_id TEXT NOT NULL,
    
    -- Transactional Data
    quantity REAL NOT NULL,
    unit_price REAL NOT NULL,        -- Snapshot of price at time of order
    
    -- KRA / Tax Compliance Data (Snapshots)
    kra_item_code TEXT,              -- From product.kra_item_code
    tax_classification_code TEXT,    -- From product.tax_classification_code (A, B, C, E)
    tax_amount REAL,                 -- Calculated: (unit_price * qty) * (tax_percent/100)
    service_state TEXT CHECK(service_state IN ('ordered', 'preparing', 'served')) DEFAULT 'ordered',

    FOREIGN KEY (order_id) REFERENCES customer_order(id) ON DELETE CASCADE,
    FOREIGN KEY (product_id) REFERENCES product(id),
    FOREIGN KEY (tax_classification_code) REFERENCES tax_classification(tax_type_code)
);


CREATE TABLE IF NOT EXISTS sale (
    id TEXT PRIMARY KEY NOT NULL,
    order_id TEXT NOT NULL,
    user_id TEXT NOT NULL,
    sale_transaction_date TEXT NOT NULL,
    net_amount REAL NOT NULL, -- total - tax(value of item)
    gross_amount REAL NOT NULL, -- sub-total + surcharge (before tax)
    tax_amount REAL NOT NULL,
    total_amount REAL NOT NULL, -- gross + tax 
    kra_receipt_number TEXT UNIQUE,
    kra_digital_signature TEXT ,
    payment_method_id INTEGER NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (order_id) REFERENCES customer_order(id),
    FOREIGN KEY (user_id) REFERENCES tbl_user(id)
    FOREIGN KEY (payment_method_id) REFERENCES payment_method(id)
    
);

CREATE TABLE IF NOT EXISTS payments (
    id TEXT PRIMARY KEY,
    order_id TEXT NOT NULL,
    payment_type TEXT NOT NULL, -- e.g., 'MPESA_STK', 'CASH'
    amount_cents INTEGER NOT NULL,
    status TEXT DEFAULT 'Initiated', -- 'Initiated', 'Success', 'Cancelled', 'Failed'
    user_tag TEXT, -- The waiter/admin who initiated it
    external_reference TEXT, -- Store CheckoutRequestID here
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS purchase (
    id TEXT PRIMARY KEY NOT NULL,
    user_id TEXT,
    purchase_date DATETIME,
    supplier_pin_number TEXT NOT NULL,
    total_invoice_amount REAL NOT NULL,
    input_tax_amount REAL NOT NULL,
    net_purchase_cost REAL NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES tbl_user(id)
);


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
    type_code INTEGER PRIMARY KEY NOT NULL,
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

CREATE TABLE IF NOT EXISTS currency (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    currency_code TEXT NOT NULL UNIQUE,
    currency_name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS country (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    country_code TEXT NOT NULL UNIQUE,
    country_name TEXT NOT NULL
);
-- =============================================================================
-- 6. SEED LOOKUPS
-- =============================================================================



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
    ('1','Raw Material'),
    ('2','Finished Product'),
    ('3','Service'),
    ('4', 'Composable');

INSERT OR IGNORE INTO stock_movement_category VALUES
    (NULL,'01','Import','IMPORT'),
    (NULL,'02','Purchase','PURCHASE'),
    (NULL,'03','Return','RETURN'),
    (NULL,'04','Stock Movement','STOCK MOVEMENT'),
    (NULL,'05','Processing','PROCESSING'),
    (NULL,'06','Adjustment','ADJUSTMENT'),
    (NULL,'11','Sale','SALE'),
    (NULL,'15','Discarding','DISCARDING');

INSERT OR IGNORE INTO country (id, country_code, country_name) VALUES
    (NULL, 'AC', 'ASCENSION ISLAND'),
    (NULL, 'AD', 'ANDORRA'),
    (NULL, 'AE', 'UNITED ARAB EMIRATES'),
    (NULL, 'AG', 'ANTIGUA AND BARBUDA'),
    (NULL, 'AI', 'ANGUILLA'),
    (NULL, 'AL', 'ALBANIA'),
    (NULL, 'AM', 'ARMENIA'),
    (NULL, 'AN', 'NETHERLANDS ANTILLES'),
    (NULL, 'AO', 'ANGOLA'),
    (NULL, 'AQ', 'ANTARCTICA'),
    (NULL, 'AR', 'ARGENTINA'),
    (NULL, 'AS', 'AMERICAN SAMOA'),
    (NULL, 'AT', 'AUSTRIA'),
    (NULL, 'AU', 'AUSTRALIA'),
    (NULL, 'AW', 'ARUBA'),
    (NULL, 'AX', 'ALAND ISLANDS'),
    (NULL, 'AZ', 'AZERBAIJAN'),
    (NULL, 'BA', 'BOSNIA AND HERZEGOVINA'),
    (NULL, 'BB', 'BARBADOS'),
    (NULL, 'BD', 'BANGLADESH'),
    (NULL, 'BE', 'BELGIUM'),
    (NULL, 'BF', 'BURKINA FASO'),
    (NULL, 'BG', 'BULGARIA'),
    (NULL, 'BH', 'BAHRAIN'),
    (NULL, 'BI', 'BURUNDI'),
    (NULL, 'BJ', 'BENIN'),
    (NULL, 'BM', 'BERMUDA'),
    (NULL, 'BN', 'BRUNEI DARUSSALAM'),
    (NULL, 'BO', 'BOLIVIA'),
    (NULL, 'BR', 'BRAZIL'),
    (NULL, 'BS', 'BAHAMAS'),
    (NULL, 'BT', 'BHUTAN'),
    (NULL, 'BV', 'BOUVET ISLAND'),
    (NULL, 'BW', 'BOTSWANA'),
    (NULL, 'BY', 'BELARUS'),
    (NULL, 'BZ', 'BELIZE'),
    (NULL, 'CA', 'CANADA'),
    (NULL, 'CC', 'COCOS (KEELING) ISLANDS'),
    (NULL, 'CD', 'CONGO, DEMOCRATIC REPUBLIC'),
    (NULL, 'CF', 'CENTRAL AFRICAN REPUBLIC'),
    (NULL, 'CG', 'CONGO'),
    (NULL, 'CH', 'SWITZERLAND'),
    (NULL, 'CI', 'COTE D''IVOIRE'),
    (NULL, 'CK', 'COOK ISLANDS'),
    (NULL, 'CL', 'CHILE'),
    (NULL, 'CM', 'CAMEROON'),
    (NULL, 'CN', 'CHINA'),
    (NULL, 'CO', 'COLOMBIA'),
    (NULL, 'CR', 'COSTA RICA'),
    (NULL, 'CS', 'CZECHOSLOVAKIA (FORMER)'),
    (NULL, 'CU', 'CUBA'),
    (NULL, 'CV', 'CAPE VERDE'),
    (NULL, 'CX', 'CHRISTMAS ISLAND'),
    (NULL, 'CY', 'CYPRUS'),
    (NULL, 'CZ', 'CZECH REPUBLIC'),
    (NULL, 'DE', 'GERMANY'),
    (NULL, 'DJ', 'DJIBOUTI'),
    (NULL, 'DK', 'DENMARK'),
    (NULL, 'DM', 'DOMINICA'),
    (NULL, 'DO', 'DOMINICAN REPUBLIC'),
    (NULL, 'DZ', 'ALGERIA'),
    (NULL, 'EC', 'ECUADOR'),
    (NULL, 'EE', 'ESTONIA'),
    (NULL, 'EG', 'EGYPT'),
    (NULL, 'EH', 'WESTERN SAHARA'),
    (NULL, 'ER', 'ERITREA'),
    (NULL, 'ES', 'SPAIN'),
    (NULL, 'ET', 'ETHIOPIA'),
    (NULL, 'EU', 'EUROPEAN UNION'),
    (NULL, 'FI', 'FINLAND'),
    (NULL, 'FJ', 'FIJI'),
    (NULL, 'FK', 'FALKLAND ISLANDS'),
    (NULL, 'FM', 'MICRONESIA'),
    (NULL, 'FO', 'FAROE ISLANDS'),
    (NULL, 'FR', 'FRANCE'),
    (NULL, 'FX', 'FRANCE, METROPOLITAN'),
    (NULL, 'GA', 'GABON'),
    (NULL, 'GB', 'GREAT BRITAIN (UK)'),
    (NULL, 'GD', 'GRENADA'),
    (NULL, 'GE', 'GEORGIA'),
    (NULL, 'GF', 'FRENCH GUIANA'),
    (NULL, 'GG', 'GUERNSEY'),
    (NULL, 'GH', 'GHANA'),
    (NULL, 'GI', 'GIBRALTAR'),
    (NULL, 'GL', 'GREENLAND'),
    (NULL, 'GM', 'GAMBIA'),
    (NULL, 'GN', 'GUINEA'),
    (NULL, 'GP', 'GUADELOUPE'),
    (NULL, 'GQ', 'EQUATORIAL GUINEA'),
    (NULL, 'GR', 'GREECE'),
    (NULL, 'GS', 'S. GEORGIA AND S. SANDWICH ISLS.'),
    (NULL, 'GT', 'GUATEMALA'),
    (NULL, 'GU', 'GUAM'),
    (NULL, 'GW', 'GUINEA BISSAU'),
    (NULL, 'GY', 'GUYANA'),
    (NULL, 'HK', 'HONG KONG'),
    (NULL, 'HM', 'HEARD AND MCDONALD ISLANDS'),
    (NULL, 'HN', 'HONDURAS'),
    (NULL, 'HR', 'CROATIA'),
    (NULL, 'HT', 'HAITI'),
    (NULL, 'HU', 'HUNGARY'),
    (NULL, 'ID', 'INDONESIA'),
    (NULL, 'IE', 'IRELAND'),
    (NULL, 'IL', 'ISRAEL'),
    (NULL, 'IM', 'ISLE OF MAN'),
    (NULL, 'IN', 'INDIA'),
    (NULL, 'IO', 'BRITISH INDIAN OCEAN TERRITORY'),
    (NULL, 'IQ', 'IRAQ'),
    (NULL, 'IR', 'IRAN'),
    (NULL, 'IS', 'ICELAND'),
    (NULL, 'IT', 'ITALY'),
    (NULL, 'JE', 'JERSEY'),
    (NULL, 'JM', 'JAMAICA'),
    (NULL, 'JO', 'JORDAN'),
    (NULL, 'JP', 'JAPAN'),
    (NULL, 'KE', 'KENYA'),
    (NULL, 'KG', 'KYRGYZSTAN'),
    (NULL, 'KH', 'CAMBODIA'),
    (NULL, 'KI', 'KIRIBATI'),
    (NULL, 'KM', 'COMOROS'),
    (NULL, 'KN', 'SAINT KITTS AND NEVIS'),
    (NULL, 'KP', 'NORTH KOREA'),
    (NULL, 'KR', 'SOUTH KOREA'),
    (NULL, 'KW', 'KUWAIT'),
    (NULL, 'KY', 'CAYMAN ISLANDS'),
    (NULL, 'KZ', 'KAZAKHSTAN'),
    (NULL, 'LA', 'LAOS'),
    (NULL, 'LB', 'LEBANON'),
    (NULL, 'LC', 'SAINT LUCIA'),
    (NULL, 'LI', 'LIECHTENSTEIN'),
    (NULL, 'LK', 'SRI LANKA'),
    (NULL, 'LR', 'LIBERIA'),
    (NULL, 'LS', 'LESOTHO'),
    (NULL, 'LT', 'LITHUANIA'),
    (NULL, 'LU', 'LUXEMBOURG'),
    (NULL, 'LV', 'LATVIA'),
    (NULL, 'LY', 'LIBYA'),
    (NULL, 'MA', 'MOROCCO'),
    (NULL, 'MC', 'MONACO'),
    (NULL, 'MD', 'MOLDOVA'),
    (NULL, 'ME', 'MONTENEGRO'),
    (NULL, 'MG', 'MADAGASCAR'),
    (NULL, 'MH', 'MARSHALL ISLANDS'),
    (NULL, 'MK', 'MACEDONIA'),
    (NULL, 'ML', 'MALI'),
    (NULL, 'MM', 'MYANMAR'),
    (NULL, 'MN', 'MONGOLIA'),
    (NULL, 'MO', 'MACAU'),
    (NULL, 'MP', 'NORTHERN MARIANA ISLANDS'),
    (NULL, 'MQ', 'MARTINIQUE'),
    (NULL, 'MR', 'MAURITANIA'),
    (NULL, 'MS', 'MONTSERRAT'),
    (NULL, 'MT', 'MALTA'),
    (NULL, 'MU', 'MAURITIUS'),
    (NULL, 'MV', 'MALDIVES'),
    (NULL, 'MW', 'MALAWI'),
    (NULL, 'MX', 'MEXICO'),
    (NULL, 'MY', 'MALAYSIA'),
    (NULL, 'MZ', 'MOZAMBIQUE'),
    (NULL, 'NA', 'NAMIBIA'),
    (NULL, 'NC', 'NEW CALEDONIA'),
    (NULL, 'NE', 'NIGER'),
    (NULL, 'NF', 'NORFOLK ISLAND'),
    (NULL, 'NG', 'NIGERIA'),
    (NULL, 'NI', 'NICARAGUA'),
    (NULL, 'NL', 'NETHERLANDS'),
    (NULL, 'NO', 'NORWAY'),
    (NULL, 'NP', 'NEPAL'),
    (NULL, 'NR', 'NAURU'),
    (NULL, 'NT', 'NEUTRAL ZONE'),
    (NULL, 'NU', 'NIUE'),
    (NULL, 'NZ', 'NEW ZEALAND'),
    (NULL, 'OM', 'OMAN'),
    (NULL, 'PA', 'PANAMA'),
    (NULL, 'PE', 'PERU'),
    (NULL, 'PF', 'FRENCH POLYNESIA'),
    (NULL, 'PG', 'PAPUA NEW GUINEA'),
    (NULL, 'PH', 'PHILIPPINES'),
    (NULL, 'PK', 'PAKISTAN'),
    (NULL, 'PL', 'POLAND'),
    (NULL, 'PM', 'ST. PIERRE AND MIQUELON'),
    (NULL, 'PN', 'PITCAIRN'),
    (NULL, 'PR', 'PUERTO RICO'),
    (NULL, 'PS', 'PALESTINIAN TERRITORY'),
    (NULL, 'PT', 'PORTUGAL'),
    (NULL, 'PW', 'PALAU'),
    (NULL, 'PY', 'PARAGUAY'),
    (NULL, 'QA', 'QATAR'),
    (NULL, 'RE', 'REUNION'),
    (NULL, 'RO', 'ROMANIA'),
    (NULL, 'RS', 'SERBIA'),
    (NULL, 'RU', 'RUSSIAN FEDERATION'),
    (NULL, 'RW', 'RWANDA'),
    (NULL, 'SA', 'SAUDI ARABIA'),
    (NULL, 'SB', 'SOLOMON ISLANDS'),
    (NULL, 'SC', 'SEYCHELLES'),
    (NULL, 'SD', 'SUDAN'),
    (NULL, 'SE', 'SWEDEN'),
    (NULL, 'SG', 'SINGAPORE'),
    (NULL, 'SH', 'ST. HELENA'),
    (NULL, 'SI', 'SLOVENIA'),
    (NULL, 'SJ', 'SVALBARD & JAN MAYEN ISLANDS'),
    (NULL, 'SK', 'SLOVAK REPUBLIC'),
    (NULL, 'SL', 'SIERRA LEONE'),
    (NULL, 'SM', 'SAN MARINO'),
    (NULL, 'SN', 'SENEGAL'),
    (NULL, 'SO', 'SOMALIA'),
    (NULL, 'SR', 'SURINAME'),
    (NULL, 'ST', 'SAO TOME AND PRINCIPE'),
    (NULL, 'SU', 'USSR (FORMER)'),
    (NULL, 'SV', 'EL SALVADOR'),
    (NULL, 'SY', 'SYRIA'),
    (NULL, 'SZ', 'SWAZILAND'),
    (NULL, 'TC', 'TURKS AND CAICOS ISLANDS'),
    (NULL, 'TD', 'CHAD'),
    (NULL, 'TF', 'FRENCH SOUTHERN TERRITORIES'),
    (NULL, 'TG', 'TOGO'),
    (NULL, 'TH', 'THAILAND'),
    (NULL, 'TJ', 'TAJIKISTAN'),
    (NULL, 'TK', 'TOKELAU'),
    (NULL, 'TM', 'TURKMENISTAN'),
    (NULL, 'TN', 'TUNISIA'),
    (NULL, 'TO', 'TONGA'),
    (NULL, 'TP', 'EAST TIMOR'),
    (NULL, 'TR', 'TURKEY'),
    (NULL, 'TT', 'TRINIDAD AND TOBAGO'),
    (NULL, 'TV', 'TUVALU'),
    (NULL, 'TW', 'TAIWAN'),
    (NULL, 'TZ', 'TANZANIA'),
    (NULL, 'UA', 'UKRAINE'),
    (NULL, 'UG', 'UGANDA'),
    (NULL, 'UK', 'UNITED KINGDOM'),
    (NULL, 'UM', 'US MINOR OUTLYING ISLANDS'),
    (NULL, 'US', 'UNITED STATES'),
    (NULL, 'UY', 'URUGUAY'),
    (NULL, 'UZ', 'UZBEKISTAN'),
    (NULL, 'VA', 'VATICAN CITY STATE'),
    (NULL, 'VC', 'SAINT VINCENT & THE GRENADINES'),
    (NULL, 'VE', 'VENEZUELA'),
    (NULL, 'VG', 'BRITISH VIRGIN ISLANDS'),
    (NULL, 'VI', 'VIRGIN ISLANDS (U.S.)'),
    (NULL, 'VN', 'VIET NAM'),
    (NULL, 'VU', 'VANUATU'),
    (NULL, 'WF', 'WALLIS AND FUTUNA ISLANDS'),
    (NULL, 'WS', 'SAMOA'),
    (NULL, 'YE', 'YEMEN'),
    (NULL, 'YT', 'MAYOTTE'),
    (NULL, 'YU', 'SERBIA AND MONTENEGRO (FORMER)'),
    (NULL, 'ZA', 'SOUTH AFRICA'),
    (NULL, 'ZM', 'ZAMBIA'),
    (NULL, 'ZW', 'ZIMBABWE');


INSERT OR IGNORE INTO currency (id, currency_code, currency_name) VALUES
    (NULL, 'AED', 'United Arab Emirates dirham'),
    (NULL, 'AFN', 'Afghan afghani'),
    (NULL, 'ALL', 'Albanian lek'),
    (NULL, 'AMD', 'Armenian dram'),
    (NULL, 'ANG', 'Netherlands Antillean guilder'),
    (NULL, 'AOA', 'Angolan kwanza'),
    (NULL, 'ARS', 'Argentine peso'),
    (NULL, 'AUD', 'Australian dollar'),
    (NULL, 'AWG', 'Aruban florin'),
    (NULL, 'AZN', 'Azerbaijani manat'),
    (NULL, 'BAM', 'Bosnia and Herzegovina convertible mark'),
    (NULL, 'BBD', 'Barbados dollar'),
    (NULL, 'BDT', 'Bangladeshi taka'),
    (NULL, 'BGN', 'Bulgarian lev'),
    (NULL, 'BHD', 'Bahraini dinar'),
    (NULL, 'BIF', 'Burundian franc'),
    (NULL, 'BMD', 'Bermudian dollar'),
    (NULL, 'BND', 'Brunei dollar'),
    (NULL, 'BOB', 'Boliviano'),
    (NULL, 'BOV', 'Bolivian Mvdol (funds code)'),
    (NULL, 'BRL', 'Brazilian real'),
    (NULL, 'BSD', 'Bahamian dollar'),
    (NULL, 'BTN', 'Bhutanese ngultrum'),
    (NULL, 'BWP', 'Botswana pula'),
    (NULL, 'BYN', 'New Belarusian ruble'),
    (NULL, 'BYR', 'Belarusian ruble'),
    (NULL, 'BZD', 'Belize dollar'),
    (NULL, 'CAD', 'Canadian dollar'),
    (NULL, 'CDF', 'Congolese franc'),
    (NULL, 'CHE', 'WIR Euro (complementary currency)'),
    (NULL, 'CHF', 'Swiss franc'),
    (NULL, 'CHW', 'WIR Franc (complementary currency)'),
    (NULL, 'CLF', 'Unidad de Fomento (funds code)'),
    (NULL, 'CLP', 'Chilean peso'),
    (NULL, 'CNY', 'Chinese yuan'),
    (NULL, 'COP', 'Colombian peso'),
    (NULL, 'COU', 'Unidad de Valor Real (UVR) (funds code)'),
    (NULL, 'CRC', 'Costa Rican colon'),
    (NULL, 'CUC', 'Cuban convertible peso'),
    (NULL, 'CUP', 'Cuban peso'),
    (NULL, 'CVE', 'Cape Verde escudo'),
    (NULL, 'CZK', 'Czech koruna'),
    (NULL, 'DJF', 'Djiboutian franc'),
    (NULL, 'DKK', 'Danish krone'),
    (NULL, 'DOP', 'Dominican peso'),
    (NULL, 'DZD', 'Algerian dinar'),
    (NULL, 'EGP', 'Egyptian pound'),
    (NULL, 'ERN', 'Eritrean nakfa'),
    (NULL, 'ETB', 'Ethiopian birr'),
    (NULL, 'EUR', 'Euro'),
    (NULL, 'FJD', 'Fiji dollar'),
    (NULL, 'FKP', 'Falkland Islands pound'),
    (NULL, 'GBP', 'Pound sterling'),
    (NULL, 'GEL', 'Georgian lari'),
    (NULL, 'GHS', 'Ghanaian cedi'),
    (NULL, 'GIP', 'Gibraltar pound'),
    (NULL, 'GMD', 'Gambian dalasi'),
    (NULL, 'GNF', 'Guinean franc'),
    (NULL, 'GTQ', 'Guatemalan quetzal'),
    (NULL, 'GYD', 'Guyanese dollar'),
    (NULL, 'HKD', 'Hong Kong dollar'),
    (NULL, 'HNL', 'Honduran lempira'),
    (NULL, 'HRK', 'Croatian kuna'),
    (NULL, 'HTG', 'Haitian gourde'),
    (NULL, 'HUF', 'Hungarian forint'),
    (NULL, 'IDR', 'Indonesian rupiah'),
    (NULL, 'ILS', 'Israeli new shekel'),
    (NULL, 'INR', 'Indian rupee'),
    (NULL, 'IQD', 'Iraqi dinar'),
    (NULL, 'IRR', 'Iranian rial'),
    (NULL, 'ISK', 'Icelandic króna'),
    (NULL, 'JMD', 'Jamaican dollar'),
    (NULL, 'JOD', 'Jordanian dinar'),
    (NULL, 'JPY', 'Japanese yen'),
    (NULL, 'KES', 'Kenyan shilling'),
    (NULL, 'KGS', 'Kyrgyzstani som'),
    (NULL, 'KHR', 'Cambodian riel'),
    (NULL, 'KMF', 'Comoro franc'),
    (NULL, 'KPW', 'North Korean won'),
    (NULL, 'KRW', 'South Korean won'),
    (NULL, 'KWD', 'Kuwaiti dinar'),
    (NULL, 'KYD', 'Cayman Islands dollar'),
    (NULL, 'KZT', 'Kazakhstani tenge'),
    (NULL, 'LAK', 'Lao kip'),
    (NULL, 'LBP', 'Lebanese pound'),
    (NULL, 'LKR', 'Sri Lankan rupee'),
    (NULL, 'LRD', 'Liberian dollar'),
    (NULL, 'LSL', 'Lesotho loti'),
    (NULL, 'LYD', 'Libyan dinar'),
    (NULL, 'MAD', 'Moroccan dirham'),
    (NULL, 'MDL', 'Moldovan leu'),
    (NULL, 'MGA', 'Malagasy ariary'),
    (NULL, 'MKD', 'Macedonian denar'),
    (NULL, 'MMK', 'Myanmar kyat'),
    (NULL, 'MNT', 'Mongolian tögrög'),
    (NULL, 'MOP', 'Macanese pataca'),
    (NULL, 'MRO', 'Mauritanian ouguiya'),
    (NULL, 'MUR', 'Mauritian rupee'),
    (NULL, 'MVR', 'Maldivian rufiyaa'),
    (NULL, 'MWK', 'Malawian kwacha'),
    (NULL, 'MXN', 'Mexican peso'),
    (NULL, 'MXV', 'Mexican Unidad de Inversion (UDI) (funds code)'),
    (NULL, 'MYR', 'Malaysian ringgit'),
    (NULL, 'MZN', 'Mozambican metical'),
    (NULL, 'NAD', 'Namibian dollar'),
    (NULL, 'NGN', 'Nigerian naira'),
    (NULL, 'NIO', 'Nicaraguan córdoba'),
    (NULL, 'NOK', 'Norwegian krone'),
    (NULL, 'NPR', 'Nepalese rupee'),
    (NULL, 'NZD', 'New Zealand dollar'),
    (NULL, 'OMR', 'Omani rial'),
    (NULL, 'PAB', 'Panamanian balboa'),
    (NULL, 'PEN', 'Peruvian Sol'),
    (NULL, 'PGK', 'Papua New Guinean kina'),
    (NULL, 'PHP', 'Philippine peso'),
    (NULL, 'PKR', 'Pakistani rupee'),
    (NULL, 'PLN', 'Polish złoty'),
    (NULL, 'PYG', 'Paraguayan guaraní'),
    (NULL, 'QAR', 'Qatari riyal'),
    (NULL, 'RON', 'Romanian leu'),
    (NULL, 'RSD', 'Serbian dinar'),
    (NULL, 'RUB', 'Russian ruble'),
    (NULL, 'RWF', 'Rwandan franc'),
    (NULL, 'SAR', 'Saudi riyal'),
    (NULL, 'SBD', 'Solomon Islands dollar'),
    (NULL, 'SCR', 'Seychelles rupee'),
    (NULL, 'SDG', 'Sudanese pound'),
    (NULL, 'SEK', 'Swedish krona'),
    (NULL, 'SGD', 'Singapore dollar'),
    (NULL, 'SHP', 'Saint Helena pound'),
    (NULL, 'SLL', 'Sierra Leonean leone'),
    (NULL, 'SOS', 'Somali shilling'),
    (NULL, 'SRD', 'Surinamese dollar'),
    (NULL, 'SSP', 'South Sudanese pound'),
    (NULL, 'STD', 'São Tomé and Príncipe dobra'),
    (NULL, 'SVC', 'Salvadoran colón'),
    (NULL, 'SYP', 'Syrian pound'),
    (NULL, 'SZL', 'Swazi lilangeni'),
    (NULL, 'THB', 'Thai baht'),
    (NULL, 'TJS', 'Tajikistani somoni'),
    (NULL, 'TMT', 'Turkmenistani manat'),
    (NULL, 'TND', 'Tunisian dinar'),
    (NULL, 'TOP', 'Tongan paʻanga'),
    (NULL, 'TRY', 'Turkish lira'),
    (NULL, 'TTD', 'Trinidad and Tobago dollar'),
    (NULL, 'TWD', 'New Taiwan dollar'),
    (NULL, 'TZS', 'Tanzanian shilling'),
    (NULL, 'UAH', 'Ukrainian hryvnia'),
    (NULL, 'UGX', 'Ugandan shilling'),
    (NULL, 'USD', 'United States dollar'),
    (NULL, 'USN', 'United States dollar (next day) (funds code)'),
    (NULL, 'UYI', 'Uruguay Peso en Unidades Indexadas (funds code)'),
    (NULL, 'UYU', 'Uruguayan peso'),
    (NULL, 'UZS', 'Uzbekistan som'),
    (NULL, 'VEF', 'Venezuelan bolívar'),
    (NULL, 'VND', 'Vietnamese dong'),
    (NULL, 'VUV', 'Vanuatu vatu'),
    (NULL, 'WST', 'Samoan tala'),
    (NULL, 'XAF', 'CFA franc BEAC'),
    (NULL, 'XAG', 'Silver (one troy ounce)'),
    (NULL, 'XAU', 'Gold (one troy ounce)'),
    (NULL, 'XBA', 'European Composite Unit (EURCO)'),
    (NULL, 'XBB', 'European Monetary Unit (E.M.U.-6)'),
    (NULL, 'XBC', 'European Unit of Account 9 (E.U.A.-9)'),
    (NULL, 'XBD', 'European Unit of Account 17 (E.U.A.-17)'),
    (NULL, 'XCD', 'East Caribbean dollar'),
    (NULL, 'XDR', 'Special drawing rights'),
    (NULL, 'XOF', 'CFA franc BCEAO'),
    (NULL, 'XPD', 'Palladium (one troy ounce)'),
    (NULL, 'XPF', 'CFP franc (franc Pacifique)'),
    (NULL, 'XPT', 'Platinum (one troy ounce)');

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
    
INSERT OR REPLACE INTO schema_info (key, value)
VALUES ('version','1.0.0'),
       ('seeded','false');

