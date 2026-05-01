"""
Convertit les fichiers Markdown du dossier docs/ en PDF.
Dépendances : markdown, xhtml2pdf (pip install markdown xhtml2pdf)
Usage : python docs/_generate_pdf.py
"""

import os
import markdown
from xhtml2pdf import pisa

CSS = """
@page {
    size: A4;
    margin: 2cm 2.5cm;
    @frame footer {
        -pdf-frame-content: footer;
        bottom: 1cm;
        height: 0.8cm;
        left: 2.5cm;
        right: 2.5cm;
    }
}
body {
    font-family: "Helvetica", "Arial", sans-serif;
    font-size: 11pt;
    line-height: 1.5;
    color: #1a1a1a;
}
h1 { font-size: 20pt; color: #1a56db; border-bottom: 2px solid #1a56db;
     padding-bottom: 4pt; margin-top: 0; }
h2 { font-size: 14pt; color: #1e429f; margin-top: 18pt; }
h3 { font-size: 12pt; color: #374151; margin-top: 12pt; }
code, pre {
    font-family: "Courier New", monospace;
    font-size: 9pt;
    background-color: #f3f4f6;
    border-radius: 3px;
}
pre {
    padding: 8pt;
    border-left: 3px solid #1a56db;
    overflow-x: auto;
    white-space: pre-wrap;
    word-wrap: break-word;
}
code { padding: 1pt 3pt; }
table {
    border-collapse: collapse;
    width: 100%;
    margin: 10pt 0;
    font-size: 10pt;
}
th {
    background-color: #1e429f;
    color: white;
    padding: 6pt 8pt;
    text-align: left;
}
td {
    padding: 5pt 8pt;
    border-bottom: 1px solid #e5e7eb;
}
tr:nth-child(even) td { background-color: #f9fafb; }
blockquote {
    border-left: 4px solid #f59e0b;
    margin: 8pt 0;
    padding: 4pt 10pt;
    background-color: #fffbeb;
    color: #92400e;
    font-style: italic;
}
a { color: #1a56db; }
#footer {
    font-size: 8pt;
    color: #9ca3af;
    text-align: center;
}
"""

DOCS_DIR = os.path.dirname(os.path.abspath(__file__))

md_files = [f for f in os.listdir(DOCS_DIR)
            if f.endswith(".md") and not f.startswith("_")]

for md_file in md_files:
    md_path  = os.path.join(DOCS_DIR, md_file)
    pdf_path = os.path.join(DOCS_DIR, md_file.replace(".md", ".pdf"))

    with open(md_path, encoding="utf-8") as f:
        md_content = f.read()

    html_body = markdown.markdown(
        md_content,
        extensions=["tables", "fenced_code", "codehilite"]
    )

    title = md_file.replace(".md", "").replace("_", " ").title()
    html = f"""<!DOCTYPE html>
<html><head>
<meta charset="utf-8">
<title>{title}</title>
<style>{CSS}</style>
</head>
<body>
{html_body}
<div id="footer">LilyGO SMS Gateway — {md_file}</div>
</body></html>"""

    with open(pdf_path, "wb") as out:
        result = pisa.CreatePDF(html, dest=out)

    if result.err:
        print(f"  ERREUR : {md_file} → {result.err}")
    else:
        size_kb = os.path.getsize(pdf_path) // 1024
        print(f"  OK : {md_file} -> {os.path.basename(pdf_path)} ({size_kb} KB)")

print("Terminé.")
