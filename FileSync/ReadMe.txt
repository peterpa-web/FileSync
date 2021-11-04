===============================================================================


/////////////////////////////////////////////////////////////////////////////


ToDo:
- Anzeige nach Copy sehr langsam bei Network drive: Scan optimieren

- DirView: ComboBoxDir bleibt gelb nach korrektem OpenDir via Icon; bleibt weiß nach Auswahl eines schlechten Dir aus der Box

- CDocDirZip: Nach Kopieren eine Datei aus einem Zip in ein neues Zip wird (modif.) nicht für das zu erstellende Zip angezeigt
-             Löschen eines expandierten ZIPs in native-Dir setzt Save-Icon

- CViewDir::ReactiveDocument Auffinden des Docs verbessern, in Tree selektieren (?)

? Abbruch bei multiple Files to R/W

? Copy Files ProgressDlg not updating

?- CViewDir Click to expand internally should only expand the selected side (or both)

?- CViewDir Copy File to r/o Fehlermeldung falsch (=0)

?- Dir: Scroll via move-bar hakelt

?- CViewText: ProgressDlg bei öffnen/vergleichen

?- UpdateHscroll m_tree.GetMaxLineLen()

.- ViewText: Nach Modif. und Save erscheint Dreieck (modif)
.- Disable change detection during/after Text saving (?)
.- Start Idle task ASSERT_VALID wg. pure virt Fkt
.- XML/Hex (?) assert beim Öffnen einer einzelnen Datei
.- CViewDir: rechter Rand franzt aus beim Expandieren von kl. Vz.
.- CViewDir VScrollBar hidden at startup sometimes
.- CViewDir: Bei Copy einer Datei mit gleichem Timestamp bleibt Anzeige blau (Size wird nicht akt.)
.- DirView disable unallowed icons, selection, etc. during ProgressDlg
.- After external create main dir: ASSERT bad sort order
o- DirView delete single subdir: not removed from view (??? not reprod.)
o- TaskDir/ThreadBack hangs(?) after switching back from ViewText single side(?)

X- DirView: Doppelklick auf Unterverzeichnis öffnet leeres Fenster (OnRefresh changed!)
X- Kein Wheel nach Save-Button in ListView
X- Ping Timeout -> 10s, bzw. Ping bei Startup?
X- Maus-Rad Unterstützung verbessern
X- Combo Eingabe \\server\c$ geht nicht, nur c$\
X- ZIP in VMWare mounted drive öffnet nicht: CDocDirZipRoot::ScanPathInt open m_cause=5 ; ziparchive.cpp(77) : ZipArchive already opened.
X- ViewDir: bei 2 sel. Zeilen und mind. 1 leeren Elem. (von 4) soll OnDoubleClick die Elem. aus beiden Zeilen vergleichen
X- ViewText: Nach Modif. und ViewDir mit Save-Cancel erfolgt Crash
X- Beep (win7)
X- ProgressDlg during ISO/ZIP unpack
X- ComboDir yellow background on error
X- confirm delete: show count of real files
X- CViewDir: Single Expand (+) beschleunigen
X- Flimmern vermeiden bei Update wenn keine Änderung im VZ inkl. Unter-VZ (no invalidate for markDirty?)
X- Timeeout 5min for hidden folder after autorefresh ähnlich CViewDir::OnDirFree()
X- CViewDir::OnUpdateTypeRW R/O - R/W bereits im Popup Menu
X- CViewDir::OnTypeRW() auch für Dirs (insb. Zip + Iso), Mehrfachselect, Toggle r/o-r/w je nach pre-Test, siehe OnUpdateEditReplacesel; Hilfe ergänzen
X- CViewDir Copy  verify old dlg: ignore dirs
X- CViewText: Find beep: Assure visible selected line
X- CViewText QuickFix Block: Ctrl-Doppel-Klick auf highlighted Linenumbers selectiert den Block und kopiert ihn auf die andere Seite
X- CViewDir: Menu View Hide *.pj CDocDirNative flag?
X- ChangeNotification: Redesign Synchr.!
X- CViewText QuickFix Line: Ctrl-Doppel-Klick auf hell markierten Text soll diese Differenz in die Gegenseite übernehmen. Entsprechend soll
  nicht hell markierter Text übernommen werden und dabei zusätzlich vorhandene Zeichen löschen.
X- CUndoDelete: Nach Del den Selektionsbereich entfernen bzw. auf 1 Zeile reduzieren.
X- CViewText: Testfiles TraceLog...: Löschen der Zeilen 1 - 2356 rechts löscht alles:
X- CView *: F3 für FindNext
X- CViewDir: Toolbar CViewText falsche Richtung nach Öffnen Datei Rechts; 
X- ChangeNotification reset überarbeiten: Pfad testen, warten? Oder View reset ?
X- CViewDir: Drag ZIP in rechte ComboBox crash wg. fehlender Side
	CComboBoxDir::DoOpen 186
	CDocDirZipRoot::OnOpenDocument 112
X- CViewDir: Nach Ret aus CViewText wird Diff nicht gesetzt wenn Datum und Size gleich waren!
X- CViewText: Unique lines markieren? line# kursiv ?
X- CViewDir: drag file to explorer window (?)
X- Copy L->R frisch\test1.zip\res\test2.zip\test1.txt wird nicht angezeigt
X- Del frisch\test1.zip\res\test2.zip - danach Save mit bad path, obwohl nicht mehr vorh.
X- ZIP ändern: unbeteiligte Ordner außerhalb ZIP werden mit Modif angezeigt
X- AutoSave-Dialog: R/L angeben
X- CDocZip: Expand von Zips aus Verzeichnis bricht ab
X- Zip im Zip kann nicht expandiert werden
X- CViewDir: Bei Rückkehr von Text View mit nur einer Datei aus Unterverzeichnis den Pfad der leeren Datei 
   besser prüfuen um unnötiges Undo zu vermeiden
X- Flackern beim externen Löschen eines großen Verzeichnisses (zB. Project\lib)
X- MRU zwischendurch speichern um Fehler-Rückkehr zu beschleuningen: CViewDir::OnRefresh
X- CViewText: Laden großer Dateien ohne Trace - Zwischenschritte einbauen
X- CViewText: Löschen von Zeilen läßt andere Seite verschwinden:
		TestR - TestL\log.txt: mittlere Zeile löschen
X- Startup MRU ZIP file no longer present gives NULL pointer
X- CViewDir deleting RO file gives ASSERT and wrong err msg
X- Asserts at close sometimes
X- CViewText boot.log hängt:
X- remove old duplicate in list at New for CTaskDirCompFiles, CTaskIdleInv
X- CThreadBack::MyThreadProc() wait: process idle msgs without wait on mouse move etc.
X- TextView: clipboard copy line(s) using shortcut as shown from menu is ignored
X- Copy files: highlighting not updated
X- DirView display size as nnn.nnn
X- ProgressDlg add time forecast and parent for file name
X- DirView allow copy/move for LNK files
X- CStorage für Tasks um die Speicherverwaltung zu optimieren
X- DirView enable EditReplace nach Unselect eines Elements
X- Draging link to ComboBoxDir
X- Copy progress
X- MainWnd statusbar update incl backthread progress
X- Suchen in Hex
X- Save As: update mode
X- C in ComboSearch ignoriert
X- EditPrefs Dialog Encoding Auswahlbox klappt nicht aus
X- SaveAs Dlg ohne OpenAs-Feld
X- Menu View: switch directly to Text, XML, Hex
X- Icons update: Save blue
X- Help: contents and F1 / (?) for each view
X- Line Edit: combine lines (Ctrl Enter)
X- colors dialog: color meaning
X- Assoc Dialog: current file type
X- ViewDir pending size ??? after manual expand
X- ViewDir: ZIP-Dir dirty not solved; equal coloring/collected size
X- Dir case sensitive separation
X- ViewText: Extra lines green
X- RO mode bei MKS nicht angezeigt
X- Exception bei Fehler in Copy to ZIP beendet Progressdialog nicht, dito bei DirNative copy error / canceled... [File deleted during copy]
X- ZIP file: adding a file and save does not update the ZIP (? subfolder)
X- ZIP extern update aktualisiert nur das Datum, nicht den Inhalt
X- Copy/Del ZIP: Tree Elemente löschen vor Verarbeitung?
X- ZIP update
X - ViewFileSync.cpp CComboBox size
X - Datei-Datum um 1h zu groß
X - Datei-Datum unmittelbar nach Copy (auf share?) um 1h falsch (?)
X- ComboDir: Beep bei Enter wenn nicht erfolgreich
X- ComboDir: desktop für tooltips beachten
X- ComboDir: lost focus im Edit Feld muss tooltip entfernen
X- DirView nach Start sind beide Seiten highlighted
X- TextView ^V in Suchfeld wird auch in Liste übernommen
?- TextView Refresh immer enable, aber Gelb bei Änderungen: ? Update Toolbar von FileChange
X ViewFileSync toolbars: magenta für OpenDir, >> statt ..., erweiterte toolbar statt Menü,
  incl. Mode R/Y/G
  Dir: Open, File, Save
  File: Open, Mode, Save [, More: SaveAs, Dir, Close]
X- DirSel Dialog bei \\server\c$\xxx warten verkürzen
X - ViewText: 2 kurze gleiche Files: Nach Eingabe am Ende wird Inhalt doppelt gezeigt
X - ViewDir: nach Copy L->R ist Dir bis auf Extras in R gleich - wird aber mit gelb ### markiert (invalidate?)
?- ViewText (+ViewDir): upd RO -> RW wird nicht immer erkannt
?- ViewDir NextDiff nicht immer zuverlässig (Differenzen werden z.T. übersprungen)
X - Select für jedes TreeElement; SetParentSel() - IsAnySel(h)
X- Tree ContextMenu: Select diffs (grün, magenta)
X- Edit line: split line
?- Nach FileOpen (via ComboDir, pDM->OnFileOpen() die entspr. Seite auswählen, damit die Diff-Selection funktioniert. SetFocus reicht nicht

- cancel long dir scan with ESC (???)
- Double click in marked (number-) area selects this
- File Save As: Neu ins Menü / Pfad anpassen???


? - Dir View upd falsch: neue Datei/Ordner wenn Gegenseite bereits vorhanden
? - Verzeichnis-Unterbaum löschen: solange Datei vorh. wird nur diese gelöscht
