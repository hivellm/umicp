/**
 * UMICP Documentation Interactive Features
 * Provides enhanced functionality for the documentation site
 */

(function() {
    'use strict';

    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }

    function init() {
        initializeThemeToggle();
        initializeTabSwitching();
        initializeSearch();
        initializeSmoothScrolling();
        initializeCodeCopyButtons();
        initializeTOCHighlighting();
        initializeMobileNavigation();
    }

    /**
     * Theme Toggle Functionality
     */
    function initializeThemeToggle() {
        // Detect system theme preference
        const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
        const savedTheme = localStorage.getItem('umicp-theme');
        const initialTheme = savedTheme || (prefersDark ? 'dark' : 'light');

        // Apply initial theme
        document.documentElement.setAttribute('data-theme', initialTheme);

        // Create theme toggle button if not exists
        let themeToggle = document.querySelector('.theme-toggle');
        if (!themeToggle) {
            themeToggle = document.createElement('button');
            themeToggle.className = 'theme-toggle';
            themeToggle.setAttribute('aria-label', 'Toggle theme');
            themeToggle.innerHTML = initialTheme === 'dark' ? '☀️' : '🌙';

            // Add to navigation
            const navLinks = document.querySelector('.nav-links');
            if (navLinks) {
                navLinks.appendChild(themeToggle);
            }
        }

        // Theme toggle handler
        themeToggle.addEventListener('click', function() {
            const currentTheme = document.documentElement.getAttribute('data-theme');
            const newTheme = currentTheme === 'dark' ? 'light' : 'dark';

            document.documentElement.setAttribute('data-theme', newTheme);
            localStorage.setItem('umicp-theme', newTheme);

            themeToggle.innerHTML = newTheme === 'dark' ? '☀️' : '🌙';
        });
    }

    /**
     * Tab Switching for Code Examples
     */
    function initializeTabSwitching() {
        const tabContainers = document.querySelectorAll('.quick-start-tabs, .code-tabs');

        tabContainers.forEach(container => {
            const buttons = container.querySelectorAll('.tab-button');
            const panes = container.querySelectorAll('.tab-pane');

            buttons.forEach(button => {
                button.addEventListener('click', function() {
                    const targetTab = this.getAttribute('data-tab');

                    // Update button states
                    buttons.forEach(btn => btn.classList.remove('active'));
                    this.classList.add('active');

                    // Update pane visibility
                    panes.forEach(pane => {
                        pane.classList.remove('active');
                        if (pane.id === targetTab) {
                            pane.classList.add('active');
                        }
                    });
                });
            });
        });
    }

    /**
     * Search Functionality
     */
    function initializeSearch() {
        // Create search interface if not exists
        let searchContainer = document.querySelector('.search-container');
        if (!searchContainer) {
            searchContainer = document.createElement('div');
            searchContainer.className = 'search-container';
            searchContainer.innerHTML = `
                <div class="search-box">
                    <input type="text" class="search-input" placeholder="Search documentation..." aria-label="Search">
                    <div class="search-results" hidden></div>
                </div>
            `;

            // Add to sidebar or header
            const sidebar = document.querySelector('.docs-sidebar');
            if (sidebar) {
                sidebar.insertBefore(searchContainer, sidebar.firstChild);
            }
        }

        const searchInput = searchContainer.querySelector('.search-input');
        const searchResults = searchContainer.querySelector('.search-results');

        if (searchInput) {
            let searchIndex = null;
            let searchTimeout = null;

            // Build search index
            buildSearchIndex().then(index => {
                searchIndex = index;
            });

            searchInput.addEventListener('input', function() {
                clearTimeout(searchTimeout);
                const query = this.value.trim().toLowerCase();

                if (query.length < 2) {
                    searchResults.hidden = true;
                    return;
                }

                searchTimeout = setTimeout(() => {
                    performSearch(query, searchIndex, searchResults);
                }, 300);
            });

            // Hide results when clicking outside
            document.addEventListener('click', function(e) {
                if (!searchContainer.contains(e.target)) {
                    searchResults.hidden = true;
                }
            });
        }
    }

    /**
     * Build search index from page content
     */
    async function buildSearchIndex() {
        const index = [];

        // Index current page content
        const headings = document.querySelectorAll('h1, h2, h3, h4, h5, h6');
        headings.forEach(heading => {
            const text = heading.textContent.trim();
            const id = heading.id || generateId(text);

            if (!heading.id) {
                heading.id = id;
            }

            index.push({
                title: text,
                content: text,
                url: `#${id}`,
                type: 'heading',
                level: parseInt(heading.tagName.charAt(1))
            });
        });

        // Index API items
        const apiItems = document.querySelectorAll('.api-card, .member-item');
        apiItems.forEach(item => {
            const title = item.querySelector('.api-name, .member-name');
            const description = item.querySelector('.api-description, .member-description');

            if (title) {
                index.push({
                    title: title.textContent.trim(),
                    content: description ? description.textContent.trim() : '',
                    url: title.querySelector('a') ? title.querySelector('a').getAttribute('href') : '#',
                    type: 'api',
                    element: item
                });
            }
        });

        return index;
    }

    /**
     * Perform search and display results
     */
    function performSearch(query, searchIndex, resultsContainer) {
        if (!searchIndex) return;

        const results = searchIndex.filter(item => {
            const titleMatch = item.title.toLowerCase().includes(query);
            const contentMatch = item.content.toLowerCase().includes(query);
            return titleMatch || contentMatch;
        }).slice(0, 10); // Limit to 10 results

        if (results.length === 0) {
            resultsContainer.innerHTML = '<div class="search-no-results">No results found</div>';
        } else {
            resultsContainer.innerHTML = results.map(result => `
                <div class="search-result">
                    <a href="${result.url}" class="search-result-link">
                        <div class="search-result-title">${highlightMatch(result.title, query)}</div>
                        <div class="search-result-content">${highlightMatch(result.content.substring(0, 100), query)}</div>
                        <div class="search-result-type">${result.type}</div>
                    </a>
                </div>
            `).join('');
        }

        resultsContainer.hidden = false;
    }

    /**
     * Highlight search matches in text
     */
    function highlightMatch(text, query) {
        const regex = new RegExp(`(${escapeRegExp(query)})`, 'gi');
        return text.replace(regex, '<mark>$1</mark>');
    }

    /**
     * Smooth Scrolling for Anchor Links
     */
    function initializeSmoothScrolling() {
        const links = document.querySelectorAll('a[href^="#"]');

        links.forEach(link => {
            link.addEventListener('click', function(e) {
                const targetId = this.getAttribute('href').substring(1);
                const targetElement = document.getElementById(targetId);

                if (targetElement) {
                    e.preventDefault();

                    const headerHeight = document.querySelector('.docs-header').offsetHeight;
                    const targetPosition = targetElement.offsetTop - headerHeight - 20;

                    window.scrollTo({
                        top: targetPosition,
                        behavior: 'smooth'
                    });

                    // Update URL without triggering scroll
                    history.pushState(null, null, `#${targetId}`);
                }
            });
        });
    }

    /**
     * Copy Code Button Functionality
     */
    function initializeCodeCopyButtons() {
        const codeBlocks = document.querySelectorAll('pre code');

        codeBlocks.forEach(codeBlock => {
            const pre = codeBlock.parentElement;

            // Create copy button
            const copyButton = document.createElement('button');
            copyButton.className = 'copy-code-button';
            copyButton.innerHTML = '📋';
            copyButton.setAttribute('aria-label', 'Copy code');
            copyButton.title = 'Copy to clipboard';

            // Position button
            pre.style.position = 'relative';
            pre.appendChild(copyButton);

            // Copy functionality
            copyButton.addEventListener('click', async function() {
                const code = codeBlock.textContent;

                try {
                    await navigator.clipboard.writeText(code);
                    this.innerHTML = '✅';
                    this.title = 'Copied!';

                    setTimeout(() => {
                        this.innerHTML = '📋';
                        this.title = 'Copy to clipboard';
                    }, 2000);
                } catch (err) {
                    // Fallback for older browsers
                    const textArea = document.createElement('textarea');
                    textArea.value = code;
                    document.body.appendChild(textArea);
                    textArea.select();
                    document.execCommand('copy');
                    document.body.removeChild(textArea);

                    this.innerHTML = '✅';
                    setTimeout(() => {
                        this.innerHTML = '📋';
                    }, 2000);
                }
            });
        });
    }

    /**
     * Table of Contents Highlighting
     */
    function initializeTOCHighlighting() {
        const tocLinks = document.querySelectorAll('.toc-list a, .nav-item');
        const headings = document.querySelectorAll('h1[id], h2[id], h3[id], h4[id], h5[id], h6[id]');

        if (tocLinks.length === 0 || headings.length === 0) return;

        const observer = new IntersectionObserver((entries) => {
            entries.forEach(entry => {
                const id = entry.target.id;
                const tocLink = document.querySelector(`a[href="#${id}"]`);

                if (tocLink) {
                    if (entry.isIntersecting) {
                        // Remove active class from all links
                        tocLinks.forEach(link => link.classList.remove('active'));
                        // Add active class to current link
                        tocLink.classList.add('active');
                    }
                }
            });
        }, {
            rootMargin: '-20% 0px -70% 0px'
        });

        headings.forEach(heading => observer.observe(heading));
    }

    /**
     * Mobile Navigation
     */
    function initializeMobileNavigation() {
        // Create mobile menu toggle if not exists
        let mobileToggle = document.querySelector('.mobile-nav-toggle');
        if (!mobileToggle) {
            mobileToggle = document.createElement('button');
            mobileToggle.className = 'mobile-nav-toggle';
            mobileToggle.innerHTML = '☰';
            mobileToggle.setAttribute('aria-label', 'Toggle navigation');

            const nav = document.querySelector('.docs-nav');
            if (nav) {
                nav.insertBefore(mobileToggle, nav.firstChild);
            }
        }

        const sidebar = document.querySelector('.docs-sidebar');

        if (mobileToggle && sidebar) {
            mobileToggle.addEventListener('click', function() {
                sidebar.classList.toggle('mobile-open');
                document.body.classList.toggle('mobile-nav-open');

                this.innerHTML = sidebar.classList.contains('mobile-open') ? '✕' : '☰';
            });

            // Close mobile nav when clicking outside
            document.addEventListener('click', function(e) {
                if (!sidebar.contains(e.target) && !mobileToggle.contains(e.target)) {
                    sidebar.classList.remove('mobile-open');
                    document.body.classList.remove('mobile-nav-open');
                    mobileToggle.innerHTML = '☰';
                }
            });
        }
    }

    /**
     * Utility Functions
     */
    function generateId(text) {
        return text.toLowerCase()
                   .replace(/[^\w\s-]/g, '')
                   .replace(/\s+/g, '-')
                   .trim();
    }

    function escapeRegExp(string) {
        return string.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    }

    // Performance monitoring
    if (window.performance && window.performance.mark) {
        window.performance.mark('docs-js-loaded');
    }

    // Export for potential external use
    window.UMICPDocs = {
        init: init,
        search: performSearch,
        theme: {
            toggle: function() {
                const themeToggle = document.querySelector('.theme-toggle');
                if (themeToggle) themeToggle.click();
            }
        }
    };

})();
