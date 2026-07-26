/* ==========================================================================
   GitHub Downloader - Interactive Application Logic (Organized Folders)
   ========================================================================== */

document.addEventListener('DOMContentLoaded', () => {

  // Initial Repository Data matching 0xSenior/GitHubDownloader
  let repoFilesData = [
    {
      id: 'folder-download',
      type: 'folder',
      name: 'download',
      expanded: true,
      children: [
        { id: 'file-dl-cpp', type: 'file', ext: 'cpp', name: 'DownloadManager.cpp', size: '14.2 KB', sizeBytes: 14230, checked: false },
        { id: 'file-dl-h', type: 'file', ext: 'h', name: 'DownloadManager.h', size: '3.4 KB', sizeBytes: 3450, checked: false }
      ]
    },
    {
      id: 'folder-filesystem',
      type: 'folder',
      name: 'filesystem',
      expanded: true,
      children: [
        { id: 'file-fs-cpp', type: 'file', ext: 'cpp', name: 'FilesystemHelper.cpp', size: '8.9 KB', sizeBytes: 8920, checked: false },
        { id: 'file-fs-h', type: 'file', ext: 'h', name: 'FilesystemHelper.h', size: '2.1 KB', sizeBytes: 2100, checked: false }
      ]
    },
    {
      id: 'folder-github',
      type: 'folder',
      name: 'github',
      expanded: true,
      children: [
        { id: 'file-gh-api', type: 'file', ext: 'cpp', name: 'GitHubApi.cpp', size: '11.4 KB', sizeBytes: 11400, checked: false },
        { id: 'file-gh-parser', type: 'file', ext: 'cpp', name: 'RepoParser.cpp', size: '6.8 KB', sizeBytes: 6800, checked: false }
      ]
    },
    {
      id: 'folder-ui',
      type: 'folder',
      name: 'ui',
      expanded: true,
      children: [
        { id: 'file-ui-renderer', type: 'file', ext: 'cpp', name: 'D2DRenderer.cpp', size: '18.6 KB', sizeBytes: 18600, checked: false },
        { id: 'file-ui-window', type: 'file', ext: 'cpp', name: 'MainWindow.cpp', size: '28.4 KB', sizeBytes: 28400, checked: false },
        { id: 'file-ui-components', type: 'file', ext: 'cpp', name: 'UIComponents.cpp', size: '22.4 KB', sizeBytes: 22400, checked: false }
      ]
    },
    {
      id: 'folder-web',
      type: 'folder',
      name: 'web',
      expanded: true,
      children: [
        { id: 'file-web-index', type: 'file', ext: 'html', name: 'index.html', size: '12.7 KB', sizeBytes: 12750, checked: false },
        { id: 'file-web-css', type: 'file', ext: 'css', name: 'style.css', size: '18.9 KB', sizeBytes: 18900, checked: false },
        { id: 'file-web-js', type: 'file', ext: 'js', name: 'app.js', size: '12.5 KB', sizeBytes: 12500, checked: false }
      ]
    },
    { id: 'file-readme', type: 'file', ext: 'md', name: 'README.md', size: '3.8 KB', sizeBytes: 3850, checked: false },
    { id: 'file-sln', type: 'file', ext: 'sln', name: 'GitHubDownloader.sln', size: '1.4 KB', sizeBytes: 1420, checked: false }
  ];

  // DOM Elements
  const treeContainer = document.getElementById('treeContainer');
  const selectedCountLabel = document.getElementById('selectedCountLabel');
  const emptyState = document.getElementById('emptyState');
  const selectedFilesList = document.getElementById('selectedFilesList');
  const selectAllBtn = document.getElementById('selectAllBtn');
  const deselectAllBtn = document.getElementById('deselectAllBtn');
  const pasteBtn = document.getElementById('pasteBtn');
  const fetchBtn = document.getElementById('fetchBtn');
  const repoUrlInput = document.getElementById('repoUrlInput');
  const downloadBtn = document.getElementById('downloadBtn');
  const statusState = document.getElementById('statusState');
  const statusDetails = document.getElementById('statusDetails');
  const logsConsole = document.getElementById('logsConsole');
  const clearLogsBtn = document.getElementById('clearLogsBtn');

  // Logs System Helper
  function addLog(message, type = 'info') {
    const row = document.createElement('div');
    row.className = `log-row ${type}`;
    const time = new Date().toLocaleTimeString();
    row.innerHTML = `<span style="color: var(--text-muted); font-size: 10px; margin-right: 6px;">[${time}]</span>${message}`;
    logsConsole.appendChild(row);
    logsConsole.scrollTop = logsConsole.scrollHeight;
    return row;
  }

  // Clear Logs Handler
  clearLogsBtn.addEventListener('click', () => {
    logsConsole.innerHTML = '<div class="log-row info">Console cleared.</div>';
  });

  // Icons Helper
  function getChevronIcon() {
    return `<svg class="tree-chevron" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><path d="M6 9l6 6 6-6"/></svg>`;
  }

  function getFolderIcon() {
    return `<svg class="tree-icon folder-icon-svg" viewBox="0 0 24 24" width="18" height="18" fill="currentColor" stroke="currentColor" stroke-width="1.5"><path d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"/></svg>`;
  }

  function getFileIconByExtension(filename) {
    const ext = filename.split('.').pop().toLowerCase();
    if (ext === 'html' || ext === 'htm') {
      return `<svg class="tree-icon" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#f97316" stroke-width="2"><polyline points="16 18 22 12 16 6"/><polyline points="8 6 2 12 8 18"/></svg>`;
    }
    if (ext === 'css') {
      return `<svg class="tree-icon" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#38bdf8" stroke-width="2"><path d="M4 6h16M4 12h16M4 18h16"/><line x1="8" y1="3" x2="6" y2="21"/><line x1="18" y1="3" x2="16" y2="21"/></svg>`;
    }
    if (ext === 'js' || ext === 'jsx' || ext === 'ts' || ext === 'tsx') {
      return `<svg class="tree-icon" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#eab308" stroke-width="2"><rect x="3" y="3" width="18" height="18" rx="4"/><path d="M10 9v6M14 9v4a2 2 0 002 2"/></svg>`;
    }
    if (['jpg', 'jpeg', 'png', 'gif', 'svg', 'webp', 'ico'].includes(ext)) {
      return `<svg class="tree-icon" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#34d399" stroke-width="2"><rect x="3" y="3" width="18" height="18" rx="3"/><circle cx="8.5" cy="8.5" r="1.5"/><polyline points="21 15 16 10 5 21"/></svg>`;
    }
    if (['json', 'yml', 'yaml'].includes(ext)) {
      return `<svg class="tree-icon" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#c084fc" stroke-width="2"><path d="M14 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V8z"/><path d="M14 2v6h6"/><path d="M10 12a1 1 0 011 1v1a1 1 0 01-1 1"/></svg>`;
    }
    return `<svg class="tree-icon file-icon-svg" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V9z"/><polyline points="13 2 13 9 20 9"/></svg>`;
  }

  function getCheckSvg() {
    return `<svg viewBox="0 0 24 24" width="12" height="12" fill="none" stroke="currentColor" stroke-width="3"><polyline points="20 6 9 17 4 12"/></svg>`;
  }

  // Sort function: Folders first, then files alphabetically
  function sortItems(items) {
    return items.sort((a, b) => {
      if (a.type === 'folder' && b.type !== 'folder') return -1;
      if (a.type !== 'folder' && b.type === 'folder') return 1;
      return a.name.localeCompare(b.name);
    });
  }

  // Render Tree Function
  function renderTree() {
    treeContainer.innerHTML = '';
    const sortedData = sortItems([...repoFilesData]);

    sortedData.forEach(item => {
      if (item.type === 'folder') {
        // Calculate folder checked status
        const allChildrenChecked = item.children.every(c => c.checked);
        const someChildrenChecked = item.children.some(c => c.checked);

        // Folder item
        const folderRow = document.createElement('div');
        folderRow.className = `tree-item ${item.expanded ? '' : 'collapsed'} ${(allChildrenChecked || someChildrenChecked) ? 'is-selected-row' : ''}`;

        folderRow.innerHTML = `
          <div class="tree-item-left">
            <span class="folder-toggle" data-id="${item.id}">${getChevronIcon()}</span>
            <div class="tree-checkbox ${allChildrenChecked ? 'checked' : (someChildrenChecked ? 'checked partial' : '')}" data-folder-id="${item.id}">
              ${allChildrenChecked || someChildrenChecked ? getCheckSvg() : ''}
            </div>
            ${getFolderIcon()}
            <span class="tree-name">${item.name}</span>
          </div>
        `;

        // Folder Toggle Click
        folderRow.querySelector('.folder-toggle').addEventListener('click', (e) => {
          e.stopPropagation();
          item.expanded = !item.expanded;
          renderTree();
        });

        // Folder Checkbox Click
        folderRow.querySelector('.tree-checkbox').addEventListener('click', (e) => {
          e.stopPropagation();
          const targetChecked = !allChildrenChecked;
          item.children.forEach(child => child.checked = targetChecked);
          updateSelectionState();
          renderTree();
        });

        treeContainer.appendChild(folderRow);

        // Children Files (Sorted alphabetically)
        if (item.expanded) {
          const sortedChildren = [...item.children].sort((a, b) => a.name.localeCompare(b.name));
          sortedChildren.forEach(child => {
            const childRow = document.createElement('div');
            childRow.className = `tree-item indent-1 ${child.checked ? 'is-selected-row' : ''}`;
            childRow.innerHTML = `
              <div class="tree-item-left">
                <div class="tree-checkbox ${child.checked ? 'checked' : ''}" data-file-id="${child.id}">
                  ${child.checked ? getCheckSvg() : ''}
                </div>
                ${getFileIconByExtension(child.name)}
                <span class="tree-name">${child.name}</span>
              </div>
              <span class="tree-size">${child.size}</span>
            `;

            childRow.addEventListener('click', () => {
              child.checked = !child.checked;
              updateSelectionState();
              renderTree();
            });

            treeContainer.appendChild(childRow);
          });
        }
      } else {
        // Root Level File (e.g. index.html)
        const fileRow = document.createElement('div');
        fileRow.className = `tree-item ${item.checked ? 'is-selected-row' : ''}`;
        const iconSvg = getFileIconByExtension(item.name);

        fileRow.innerHTML = `
          <div class="tree-item-left">
            <div class="tree-checkbox ${item.checked ? 'checked' : ''}" data-file-id="${item.id}">
              ${item.checked ? getCheckSvg() : ''}
            </div>
            ${iconSvg}
            <span class="tree-name">${item.name}</span>
          </div>
          <span class="tree-size">${item.size}</span>
        `;

        fileRow.addEventListener('click', () => {
          item.checked = !item.checked;
          updateSelectionState();
          renderTree();
        });

        treeContainer.appendChild(fileRow);
      }
    });
  }

  // Get All Files Flat List
  function getAllFiles() {
    const files = [];
    repoFilesData.forEach(item => {
      if (item.type === 'folder') {
        item.children.forEach(c => files.push(c));
      } else {
        files.push(item);
      }
    });
    return files;
  }

  // Update Selection Summary
  function updateSelectionState() {
    const allFiles = getAllFiles();
    const selectedFiles = allFiles.filter(f => f.checked);
    const count = selectedFiles.length;

    if (selectedCountLabel) {
      selectedCountLabel.textContent = `${count} files selected`;
    }

    if (count === 0) {
      if (!emptyState.classList.contains('hidden')) return;
      
      // Animate entry of empty state
      emptyState.classList.remove('hidden');
      emptyState.classList.remove('fade-out');
      emptyState.classList.add('fade-in');
      selectedFilesList.classList.add('hidden');
    } else {
      emptyState.classList.add('hidden');
      selectedFilesList.classList.remove('hidden');

      selectedFilesList.innerHTML = '';
      selectedFiles.forEach(file => {
        const row = document.createElement('div');
        row.className = 'selected-file-row';
        row.innerHTML = `
          <span class="tree-name">${file.name}</span>
          <span class="tree-size">${file.size}</span>
        `;
        selectedFilesList.appendChild(row);
      });
    }
  }

  // Select All Handler
  selectAllBtn.addEventListener('click', () => {
    repoFilesData.forEach(item => {
      if (item.type === 'folder') {
        item.children.forEach(c => c.checked = true);
      } else {
        item.checked = true;
      }
    });
    updateSelectionState();
    renderTree();
    addLog('Selected all files.', 'info');
  });

  // Deselect All Handler
  deselectAllBtn.addEventListener('click', () => {
    repoFilesData.forEach(item => {
      if (item.type === 'folder') {
        item.children.forEach(c => c.checked = false);
      } else {
        item.checked = false;
      }
    });
    updateSelectionState();
    renderTree();
    addLog('Deselected all files.', 'info');
  });

  // Paste Button Handler
  pasteBtn.addEventListener('click', async () => {
    try {
      const text = await navigator.clipboard.readText();
      if (text) {
        repoUrlInput.value = text;
        addLog(`Pasted URL from clipboard: ${text}`, 'info');
      }
    } catch (err) {
      repoUrlInput.select();
      addLog('Could not access clipboard, auto-selected input text.', 'warning');
    }
  });

  // Fetch Button Simulation
  fetchBtn.addEventListener('click', () => {
    if (fetchBtn.classList.contains('fetching')) return;
    
    fetchBtn.classList.add('fetching');
    const originalText = fetchBtn.querySelector('span').textContent;
    fetchBtn.querySelector('span').textContent = 'Fetching...';
    statusState.textContent = 'Fetching Repository...';
    
    addLog(`Initializing connection to repository: ${repoUrlInput.value}...`, 'info');
    
    setTimeout(() => {
      addLog('Successfully connected. Querying repository tree structure...', 'info');
      setTimeout(() => {
        const files = getAllFiles();
        addLog(`Successfully loaded repository structure: ${files.length} files found.`, 'success');
        
        fetchBtn.classList.remove('fetching');
        fetchBtn.querySelector('span').textContent = originalText;
        statusState.textContent = 'Ready';
        statusDetails.textContent = `Loaded ${files.length} files (721.2 KB)`;
      }, 500);
    }, 600);
  });

  // Download Button Handler
  downloadBtn.addEventListener('click', () => {
    const selectedFiles = getAllFiles().filter(f => f.checked);
    if (selectedFiles.length === 0) {
      addLog('Download aborted: No files selected.', 'error');
      alert('Please select at least one file or folder to download.');
      return;
    }

    addLog(`Initializing batch download of ${selectedFiles.length} file(s)...`, 'info');
    const btnText = downloadBtn.querySelector('span');
    const originalText = btnText.textContent;
    btnText.textContent = `Downloading ${selectedFiles.length} files...`;
    downloadBtn.style.opacity = '0.8';
    downloadBtn.disabled = true;

    // Simulate each file download in logs console
    let completedCount = 0;
    
    selectedFiles.forEach((file, index) => {
      const progressItem = document.createElement('div');
      progressItem.className = 'log-progress-item';
      progressItem.innerHTML = `
        <div class="progress-info-row">
          <span class="progress-file-name">${file.name}</span>
          <span class="progress-percentage" id="pct-${file.id}">0%</span>
        </div>
        <div class="progress-bar-container">
          <div class="progress-bar-fill" id="bar-${file.id}"></div>
        </div>
        <div class="progress-info-row" style="margin-top: 2px;">
          <span id="speed-${file.id}">Connecting...</span>
          <span>Size: ${file.size}</span>
        </div>
      `;
      logsConsole.appendChild(progressItem);
      logsConsole.scrollTop = logsConsole.scrollHeight;

      let pct = 0;
      const duration = 800 + Math.random() * 1200;
      const stepTime = duration / 10;
      const speed = (2.1 + Math.random() * 4).toFixed(1);
      
      const interval = setInterval(() => {
        pct += 10;
        if (pct >= 100) {
          pct = 100;
          clearInterval(interval);
          document.getElementById(`pct-${file.id}`).textContent = 'Completed';
          document.getElementById(`pct-${file.id}`).style.color = '#34d399';
          document.getElementById(`speed-${file.id}`).textContent = `Finished in ${(duration/1000).toFixed(1)}s`;
          
          addLog(`Download finished: ${file.name}`, 'success');
          completedCount++;
          
          if (completedCount === selectedFiles.length) {
            btnText.textContent = originalText;
            downloadBtn.style.opacity = '1';
            downloadBtn.disabled = false;
            addLog(`All ${selectedFiles.length} files downloaded successfully to folder!`, 'success');
            statusState.textContent = 'Ready';
            statusDetails.textContent = `Downloaded ${selectedFiles.length} files.`;
          }
        } else {
          document.getElementById(`pct-${file.id}`).textContent = `${pct}%`;
          document.getElementById(`bar-${file.id}`).style.width = `${pct}%`;
          document.getElementById(`speed-${file.id}`).textContent = `${speed} MB/s`;
        }
      }, stepTime);
    });
  });

  // Initial Render
  renderTree();
  updateSelectionState();
});
