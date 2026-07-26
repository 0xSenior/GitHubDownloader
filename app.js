/* ==========================================================================
   GitHub Downloader - Interactive Application Logic
   ========================================================================== */

document.addEventListener('DOMContentLoaded', () => {

  // Initial Repository Data matching screenshot
  let repoFilesData = [
    {
      id: 'folder-css',
      type: 'folder',
      name: 'css',
      expanded: true,
      children: [
        { id: 'file-bootstrap-css', type: 'file', ext: 'css', name: 'bootstrap.min.css', size: '226.7 KB', sizeBytes: 232140, checked: false },
        { id: 'file-media-css', type: 'file', ext: 'css', name: 'media.css', size: '259 B', sizeBytes: 259, checked: false },
        { id: 'file-style-css', type: 'file', ext: 'css', name: 'style.css', size: '2.3 KB', sizeBytes: 2355, checked: false }
      ]
    },
    {
      id: 'folder-images',
      type: 'folder',
      name: 'images',
      expanded: true,
      children: [
        { id: 'file-contact-jpg', type: 'file', ext: 'jpg', name: 'contact-image.jpg', size: '174.3 KB', sizeBytes: 178483, checked: false },
        { id: 'file-contacts-book-png', type: 'file', ext: 'png', name: 'contacts-book.png', size: '1.3 KB', sizeBytes: 1331, checked: false },
        { id: 'file-navbar-icon-jpg', type: 'file', ext: 'jpg', name: 'navbar-icon.jpg', size: '206.1 KB', sizeBytes: 211046, checked: false }
      ]
    },
    {
      id: 'folder-js',
      type: 'folder',
      name: 'js',
      expanded: true,
      children: [
        { id: 'file-bootstrap-js', type: 'file', ext: 'js', name: 'bootstrap.bundle.min.js', size: '78.6 KB', sizeBytes: 80486, checked: false },
        { id: 'file-main-js', type: 'file', ext: 'js', name: 'main.js', size: '11.9 KB', sizeBytes: 12185, checked: false }
      ]
    },
    { id: 'file-index-html', type: 'file', ext: 'html', name: 'index.html', size: '19.7 KB', sizeBytes: 20172, checked: false }
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

  // Icons Helper
  function getChevronIcon() {
    return `<svg class="tree-chevron" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><path d="M6 9l6 6 6-6"/></svg>`;
  }

  function getFolderIcon() {
    return `<svg class="tree-icon folder-icon-svg" viewBox="0 0 24 24" width="18" height="18" fill="currentColor" stroke="currentColor" stroke-width="1.5"><path d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"/></svg>`;
  }

  function getCodeIcon() {
    return `<svg class="tree-icon code-icon-svg" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" stroke-width="2"><polyline points="16 18 22 12 16 6"/><polyline points="8 6 2 12 8 18"/></svg>`;
  }

  function getFileIcon() {
    return `<svg class="tree-icon file-icon-svg" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="currentColor" stroke-width="2"><path d="M13 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V9z"/><polyline points="13 2 13 9 20 9"/></svg>`;
  }

  function getCheckSvg() {
    return `<svg viewBox="0 0 24 24" width="12" height="12" fill="none" stroke="currentColor" stroke-width="3"><polyline points="20 6 9 17 4 12"/></svg>`;
  }

  // Render Tree Function
  function renderTree() {
    treeContainer.innerHTML = '';

    repoFilesData.forEach(item => {
      if (item.type === 'folder') {
        // Folder item
        const folderRow = document.createElement('div');
        folderRow.className = `tree-item ${item.expanded ? '' : 'collapsed'}`;
        
        // Calculate folder checked status
        const allChildrenChecked = item.children.every(c => c.checked);
        const someChildrenChecked = item.children.some(c => c.checked);

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

        // Children Files
        if (item.expanded) {
          item.children.forEach(child => {
            const childRow = document.createElement('div');
            childRow.className = 'tree-item indent-1';
            childRow.innerHTML = `
              <div class="tree-item-left">
                <div class="tree-checkbox ${child.checked ? 'checked' : ''}" data-file-id="${child.id}">
                  ${child.checked ? getCheckSvg() : ''}
                </div>
                ${getFileIcon()}
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
        fileRow.className = 'tree-item';
        const iconSvg = item.ext === 'html' ? getCodeIcon() : getFileIcon();

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

    selectedCountLabel.textContent = `${count} files selected`;

    if (count === 0) {
      emptyState.classList.remove('hidden');
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
  });

  // Paste Button Handler
  pasteBtn.addEventListener('click', async () => {
    try {
      const text = await navigator.clipboard.readText();
      if (text) {
        repoUrlInput.value = text;
      }
    } catch (err) {
      console.log('Clipboard access unavailable, focus input to paste');
      repoUrlInput.select();
    }
  });

  // Fetch Button Simulation
  fetchBtn.addEventListener('click', () => {
    const originalText = fetchBtn.querySelector('span').textContent;
    fetchBtn.querySelector('span').textContent = 'Fetching...';
    statusState.textContent = 'Fetching Repository...';
    
    setTimeout(() => {
      fetchBtn.querySelector('span').textContent = originalText;
      statusState.textContent = 'Ready';
      statusDetails.textContent = 'Loaded 9 files (721.2 KB)';
    }, 800);
  });

  // Download Button Handler
  downloadBtn.addEventListener('click', () => {
    const selectedFiles = getAllFiles().filter(f => f.checked);
    if (selectedFiles.length === 0) {
      alert('Please select at least one file or folder to download.');
      return;
    }

    const btnText = downloadBtn.querySelector('span');
    btnText.textContent = `Downloading ${selectedFiles.length} files...`;
    downloadBtn.style.opacity = '0.8';

    setTimeout(() => {
      btnText.textContent = 'Download Selected Files';
      downloadBtn.style.opacity = '1';
      alert(`Successfully downloaded ${selectedFiles.length} selected files to target location!`);
    }, 1200);
  });

  // Initial Render
  renderTree();
  updateSelectionState();
});
